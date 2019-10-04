/* FDT machine description */
#include "config.h"
#include "fdt.h"
#include <inttypes.h>
#include <stdio.h>
#include "support_io.h"
#include <memory.h>
#include <stdarg.h>
#include "memory_utils.h"
#include <stdlib.h>

#define FDT_MAGIC	0xd00dfeed
#define FDT_VERSION	17



struct fdt_header {
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_dt_struct;
	uint32_t off_dt_strings;
	uint32_t off_mem_rsvmap;
	uint32_t version;
	uint32_t last_comp_version; /* <= 17 */
	uint32_t boot_cpuid_phys;
	uint32_t size_dt_strings;
	uint32_t size_dt_struct;
};

struct fdt_reserve_entry {
	uint64_t address;
	uint64_t size;
};

#define FDT_BEGIN_NODE	1
#define FDT_END_NODE	2
#define FDT_PROP	3
#define FDT_NOP		4
#define FDT_END		9

typedef struct {
	uint32_t* tab;
	int tab_len;
	int tab_size;
	int open_node_count;

	char* string_table;
	int string_table_len;
	int string_table_size;
} FDTState;

static FDTState* fdt_init(void)
{
	FDTState* s;
	s = mallocz(sizeof(*s));
	return s;
}

static inline int max_int(int a, int b)
{
	if (a > b)
		return a;
	else
		return b;
}


static void fdt_alloc_len(FDTState* s, int len)
{
	int new_size;
	if (len > s->tab_size) {
		new_size = max_int(len, s->tab_size * 3 / 2);
		s->tab = realloc(s->tab, new_size * sizeof(uint32_t));
		s->tab_size = new_size;
	}
}

static inline uint32_t bswap_32(uint32_t v)
{
	return ((v & 0xff000000) >> 24) | ((v & 0x00ff0000) >> 8) |
		((v & 0x0000ff00) << 8) | ((v & 0x000000ff) << 24);
}

static inline uint32_t cpu_to_be32(uint32_t v)
{
	return bswap_32(v);
}

static void fdt_put32(FDTState * s, int v)
{
	fdt_alloc_len(s, s->tab_len + 1);
	s->tab[s->tab_len++] = cpu_to_be32(v);
}

/* the data is zero padded */
static void fdt_put_data(FDTState * s, const uint8_t * data, int len)
{
	int len1;

	len1 = (len + 3) / 4;
	fdt_alloc_len(s, s->tab_len + len1);
	memcpy(s->tab + s->tab_len, data, len);
	memset((uint8_t*)(s->tab + s->tab_len) + len, 0, -len & 3);
	s->tab_len += len1;
}

static void fdt_begin_node(FDTState * s, const char* name)
{
	fdt_put32(s, FDT_BEGIN_NODE);
	fdt_put_data(s, (uint8_t*)name, strlen(name) + 1);
	s->open_node_count++;
}

static void fdt_begin_node_num(FDTState * s, const char* name, uint64_t n)
{
	char buf[256];
	snprintf(buf, sizeof(buf), "%s@%" PRIx64, name, n);
	fdt_begin_node(s, buf);
}

static void fdt_end_node(FDTState * s)
{
	fdt_put32(s, FDT_END_NODE);
	s->open_node_count--;
}

static int fdt_get_string_offset(FDTState * s, const char* name)
{
	int pos, new_size, name_size, new_len;

	pos = 0;
	while (pos < s->string_table_len) {
		if (!strcmp(s->string_table + pos, name))
			return pos;
		pos += strlen(s->string_table + pos) + 1;
	}
	/* add a new string */
	name_size = strlen(name) + 1;
	new_len = s->string_table_len + name_size;
	if (new_len > s->string_table_size) {
		new_size = max_int(new_len, s->string_table_size * 3 / 2);
		s->string_table = realloc(s->string_table, new_size);
		s->string_table_size = new_size;
	}
	pos = s->string_table_len;
	memcpy(s->string_table + pos, name, name_size);
	s->string_table_len = new_len;
	return pos;
}

static void fdt_prop(FDTState * s, const char* prop_name,
	const void* data, int data_len)
{
	fdt_put32(s, FDT_PROP);
	fdt_put32(s, data_len);
	fdt_put32(s, fdt_get_string_offset(s, prop_name));
	fdt_put_data(s, data, data_len);
}

static void fdt_prop_tab_u32(FDTState * s, const char* prop_name,
	uint32_t * tab, int tab_len)
{
	int i;
	fdt_put32(s, FDT_PROP);
	fdt_put32(s, tab_len * sizeof(uint32_t));
	fdt_put32(s, fdt_get_string_offset(s, prop_name));
	for (i = 0; i < tab_len; i++)
		fdt_put32(s, tab[i]);
}

static void fdt_prop_u32(FDTState * s, const char* prop_name, uint32_t val)
{
	fdt_prop_tab_u32(s, prop_name, &val, 1);
}

static void fdt_prop_tab_u64(FDTState * s, const char* prop_name,
	uint64_t v0)
{
	uint32_t tab[2];
	tab[0] = v0 >> 32;
	tab[1] = v0;
	fdt_prop_tab_u32(s, prop_name, tab, 2);
}

static void fdt_prop_tab_u64_2(FDTState * s, const char* prop_name,
	uint64_t v0, uint64_t v1)
{
	uint32_t tab[4];
	tab[0] = v0 >> 32;
	tab[1] = v0;
	tab[2] = v1 >> 32;
	tab[3] = v1;
	fdt_prop_tab_u32(s, prop_name, tab, 4);
}

static void fdt_prop_str(FDTState * s, const char* prop_name,
	const char* str)
{
	fdt_prop(s, prop_name, str, strlen(str) + 1);
}

/* NULL terminated string list */
static void fdt_prop_tab_str(FDTState * s, const char* prop_name,
	...)
{
	va_list ap;
	int size, str_size;
	char* ptr, * tab;

	va_start(ap, prop_name);
	size = 0;
	for (;;) {
		ptr = va_arg(ap, char*);
		if (!ptr)
			break;
		str_size = strlen(ptr) + 1;
		size += str_size;
	}
	va_end(ap);

	tab = malloc(size);
	va_start(ap, prop_name);
	size = 0;
	for (;;) {
		ptr = va_arg(ap, char*);
		if (!ptr)
			break;
		str_size = strlen(ptr) + 1;
		memcpy(tab + size, ptr, str_size);
		size += str_size;
	}
	va_end(ap);

	fdt_prop(s, prop_name, tab, size);
	free(tab);
}


/* write the FDT to 'dst1'. return the FDT size in bytes */
int fdt_output(FDTState * s, uint8_t * dst)
{
	struct fdt_header* h;
	struct fdt_reserve_entry* re;
	int dt_struct_size;
	int dt_strings_size;
	int pos;

	//assert(s->open_node_count == 0);

	fdt_put32(s, FDT_END);

	dt_struct_size = s->tab_len * sizeof(uint32_t);
	dt_strings_size = s->string_table_len;

	h = (struct fdt_header*)dst;
	h->magic = cpu_to_be32(FDT_MAGIC);
	h->version = cpu_to_be32(FDT_VERSION);
	h->last_comp_version = cpu_to_be32(16);
	h->boot_cpuid_phys = cpu_to_be32(0);
	h->size_dt_strings = cpu_to_be32(dt_strings_size);
	h->size_dt_struct = cpu_to_be32(dt_struct_size);

	pos = sizeof(struct fdt_header);

	h->off_dt_struct = cpu_to_be32(pos);
	memcpy(dst + pos, s->tab, dt_struct_size);
	pos += dt_struct_size;

	/* align to 8 */
	while ((pos & 7) != 0) {
		dst[pos++] = 0;
	}
	h->off_mem_rsvmap = cpu_to_be32(pos);
	re = (struct fdt_reserve_entry*)(dst + pos);
	re->address = 0; /* no reserved entry */
	re->size = 0;
	pos += sizeof(struct fdt_reserve_entry);

	h->off_dt_strings = cpu_to_be32(pos);
	memcpy(dst + pos, s->string_table, dt_strings_size);
	pos += dt_strings_size;

	/* align to 8, just in case */
	while ((pos & 7) != 0) {
		dst[pos++] = 0;
	}

	h->totalsize = cpu_to_be32(pos);
	return pos;
}

void fdt_end(FDTState * s)
{
	free(s->tab);
	free(s->string_table);
	free(s);
}

int riscv_load_fdt(char* filename, uint8_t* dst) {
	//emuriscv_dts.bin
	//todo read bin
	int size;
	uint8_t* dts = read_bin(filename, &size);
	memcpy(dst, dts, size);
	return size;
}

int riscv_build_fdt(RiscVMachine * m, uint8_t * dst,
	uint64_t kernel_start, uint64_t kernel_size,
	const char* cmd_line)
{
	FDTState* s;
	int size, max_xlen, i, cur_phandle, intc_phandle, plic_phandle;
	char isa_string[128], * q;
	uint32_t misa;
	uint32_t tab[4];
	//FBDevice* fb_dev;

	s = fdt_init();

	cur_phandle = 1;

	fdt_begin_node(s, "");
	fdt_prop_u32(s, "#address-cells", 2);
	fdt_prop_u32(s, "#size-cells", 2);
	fdt_prop_str(s, "compatible", "ucbbar,riscvemu-bar_dev");
	fdt_prop_str(s, "model", "ucbbar,riscvemu-bare");

	/* CPU list */
	fdt_begin_node(s, "cpus");
	fdt_prop_u32(s, "#address-cells", 1);
	fdt_prop_u32(s, "#size-cells", 0);
#define RTC_FREQ 10000000
	fdt_prop_u32(s, "timebase-frequency", RTC_FREQ);

	/* cpu */
	fdt_begin_node_num(s, "cpu", 0);
	fdt_prop_str(s, "device_type", "cpu");
	fdt_prop_u32(s, "reg", 0);
	fdt_prop_str(s, "status", "okay");
	fdt_prop_str(s, "compatible", "riscv");

	max_xlen = m->max_xlen;
	//misa = riscv_cpu_get_misa(m->cpu_state);
	///q = isa_string;

	/*q += snprintf(isa_string, sizeof(isa_string), "rv%d", max_xlen);
	for (i = 0; i < 26; i++) {
		if (misa & (1 << i))
			* q++ = 'a' + i;
	}*/
	//*q = '\0';
	fdt_prop_str(s, "riscv,isa", "rv32i");

	fdt_prop_str(s, "mmu-type", max_xlen <= 32 ? "riscv,sv32" : "riscv,sv48");
	fdt_prop_u32(s, "clock-frequency", 1000000000 /*2000000000*/);

	fdt_begin_node(s, "interrupt-controller");
	fdt_prop_u32(s, "#interrupt-cells", 1);
	fdt_prop(s, "interrupt-controller", NULL, 0);
	fdt_prop_str(s, "compatible", "riscv,cpu-intc");
	intc_phandle = cur_phandle++;
	fdt_prop_u32(s, "phandle", intc_phandle);
	fdt_end_node(s); /* interrupt-controller */

	fdt_end_node(s); /* cpu */

	fdt_end_node(s); /* cpus */

	fdt_begin_node_num(s, "memory", RAM_BASE_ADDR);
	fdt_prop_str(s, "device_type", "memory");
	tab[0] = (uint64_t)RAM_BASE_ADDR >> 32;
	tab[1] = RAM_BASE_ADDR;
	tab[2] = (uint64_t)m->ram_size >> 32;
	tab[3] = m->ram_size;
	fdt_prop_tab_u32(s, "reg", tab, 4);
	fdt_end_node(s); /* memory */

	//HACK - second memory
#ifdef HACK_SECOND_MEMORY_2G
	uint32_t new_base = 0xc0000000;
	fdt_begin_node_num(s, "memory", new_base);
	fdt_prop_str(s, "device_type", "memory");
	tab[0] = (uint64_t)new_base >> 32;
	tab[1] = new_base;
	tab[2] = (uint64_t)m->ram_size >> 32;
	tab[3] = m->ram_size;
	fdt_prop_tab_u32(s, "reg", tab, 4);
	fdt_end_node(s); /* memory */

	fdt_begin_node(s, "htif");
	fdt_prop_str(s, "compatible", "ucb,htif0");
	fdt_end_node(s); /* htif */
#endif

#if 1
	fdt_begin_node(s, "soc");
	fdt_prop_u32(s, "#address-cells", 2);
	fdt_prop_u32(s, "#size-cells", 2);
	fdt_prop_tab_str(s, "compatible",
		"ucbbar,riscvemu-bar-soc", "simple-bus", NULL);
	fdt_prop(s, "ranges", NULL, 0);

	fdt_begin_node_num(s, "clint", CLINT_BASE_ADDR);
	fdt_prop_str(s, "compatible", "riscv,clint0");

	tab[0] = intc_phandle;
	tab[1] = 3; /* M IPI irq */
	tab[2] = intc_phandle;
	tab[3] = 7; /* M timer irq */
	fdt_prop_tab_u32(s, "interrupts-extended", tab, 4);

	fdt_prop_tab_u64_2(s, "reg", CLINT_BASE_ADDR, CLINT_SIZE);

	fdt_end_node(s); /* clint */
#endif

#if 0
	fdt_begin_node_num(s, "plic", PLIC_BASE_ADDR);
	fdt_prop_u32(s, "#interrupt-cells", 1);
	fdt_prop(s, "interrupt-controller", NULL, 0);
	fdt_prop_str(s, "compatible", "riscv,plic0");
	fdt_prop_u32(s, "riscv,ndev", 31);
	fdt_prop_tab_u64_2(s, "reg", PLIC_BASE_ADDR, PLIC_SIZE);

	tab[0] = intc_phandle;
	tab[1] = 9; /* S ext irq */
	tab[2] = intc_phandle;
	tab[3] = 11; /* M ext irq */
	fdt_prop_tab_u32(s, "interrupts-extended", tab, 4);

	plic_phandle = cur_phandle++;
	fdt_prop_u32(s, "phandle", plic_phandle);

	fdt_end_node(s); /* plic */
#endif

#if 0
	for (i = 0; i < m->virtio_count; i++) {
		fdt_begin_node_num(s, "virtio", VIRTIO_BASE_ADDR + i * VIRTIO_SIZE);
		fdt_prop_str(s, "compatible", "virtio,mmio");
		fdt_prop_tab_u64_2(s, "reg", VIRTIO_BASE_ADDR + i * VIRTIO_SIZE,
			VIRTIO_SIZE);
		tab[0] = plic_phandle;
		tab[1] = VIRTIO_IRQ + i;
		fdt_prop_tab_u32(s, "interrupts-extended", tab, 2);
		fdt_end_node(s); /* virtio */
	}
	

	fb_dev = m->common.fb_dev;
	if (fb_dev) {
		fdt_begin_node_num(s, "framebuffer", FRAMEBUFFER_BASE_ADDR);
		fdt_prop_str(s, "compatible", "simple-framebuffer");
		fdt_prop_tab_u64_2(s, "reg", FRAMEBUFFER_BASE_ADDR, fb_dev->fb_size);
		fdt_prop_u32(s, "width", fb_dev->width);
		fdt_prop_u32(s, "height", fb_dev->height);
		fdt_prop_u32(s, "stride", fb_dev->stride);
		fdt_prop_str(s, "format", "a8r8g8b8");
		fdt_end_node(s); //framebuffer
	}
#endif	

	//fdt_end_node(s); /* soc */

	fdt_begin_node(s, "chosen");
	fdt_prop_str(s, "bootargs", cmd_line ? cmd_line : "");
	if (kernel_size > 0) {
		fdt_prop_tab_u64(s, "riscv,kernel-start", kernel_start);
		fdt_prop_tab_u64(s, "riscv,kernel-end", kernel_start + kernel_size);
	}

	fdt_end_node(s); /* chosen */

	fdt_end_node(s); /* / */

	size = fdt_output(s, dst);
//#if 0
	{
		FILE* f;
		f = fopen("c:\\temp\\riscvemu.fdt", "wb");
		fwrite(dst, 1, size, f);
		fclose(f);
	}
//#endif
	fdt_end(s);
	return size;
}