#include "memory.h"
#include <stdio.h>
#include "csr.h"

//convenience function to obtain the physical address
//uint8_t* get_physical_address(State* state, uint32_t virtual_address) {
//	MemoryTarget target = get_memory_target(state, virtual_address);
//	return target.ptr;
//}

//word* fetch_next_word(State* state) {
//	MemoryTarget target = get_memory_target(state, state->pc);
//	state->pc += 4;
//	return target.ptr;
//}

void write_common_ram(State* state, uint8_t* target, word value, int size_log2) {
	switch (size_log2) {
	case SIZE_WORD:
		*(target + 3) = (value & 0xff000000) >> 24;
		*(target + 2) = (value & 0xff0000) >> 16;
	case SIZE_HALF:
		*(target + 1) = (value & 0xff00) >> 8;
	case SIZE_BYTE:
		*target = value & 0xff;
		break;
	default:
		fprintf(stderr, "write_common_ram unsupported size:%d", size_log2);
		break;
	}
}

void write_word_physical(State* state, word physical_address, word value) {
	MemoryTarget target;
	int status = get_memory_target_physical(state, physical_address, &target);
	write_common_ram(state, target.ptr, value, SIZE_WORD);
}

int get_memory_mode(State* state) {
	return read_csr(state, CSR_SATP) >> 31;
}

//MemoryTarget get_memory_target_bare(State * state, word address, MemoryTarget *target) {
//	MemoryTarget target;
//	uint8_t* ptr;
//	uint32_t target_address = address;
//	target->range = get_phys_mem_range(state->memory_map, target_address);
//
//	if (!target->range) {
//		printf("get_physical_address: invalid physical address 0x%08x\n", target_address);
//		exit(1);
//		return -2;
//	}
//	target->ptr = target->range->phys_mem_ptr + (uint32_t)(target_address - target->range->address);
//	return target;
//}

int get_memory_target_physical(State* state, word physical_address, MemoryTarget* target) {
	//MemoryTarget target;
	target->range = get_phys_mem_range(state->memory_map, physical_address);

	if (!target->range) {
		printf("get_memory_target_physical: invalid physical address 0x%08x\n", physical_address);
		//exit(1);
		return -2;
	}
	word range_offset = (uint32_t)(physical_address - target->range->address);
	target->ptr = target->range->phys_mem_ptr + range_offset;
	return TRANSLATE_OK;
}

word read_word_physical(State* state, word physical_address) {
	MemoryTarget target;
	int status = get_memory_target_physical(state, physical_address, &target);
	return read_common_ram(state, target.ptr, SIZE_WORD);
}

#define PTE_W 2
#define PTE_XW 6

//returns 0 on success, negative value on exception
int translate_address(State * state, word virtual_address, enum access_type access_type, word * physical_address) {
	//in machine mode no translation is happening
	if (state->privilege == PRIV_M) {
		*physical_address = virtual_address;
		return TRANSLATE_OK;
	}
	//supervisor mode
	int memory_mode = get_memory_mode(state);
	if (memory_mode == SATP_MODE_BARE) {
		*physical_address = virtual_address;
		return TRANSLATE_OK;
	}
	else if (memory_mode == SATP_MODE_SV32) {
		//TODO it would be a good idea to decode satp into a structure
		//An Sv32 virtual address  is  partitioned  into  a virtual  page  number  (VPN)  and  page  offset,
		//bits 0..11=offset, 12..21=vpn[0], 22..31=vpn[1]
		word satp = read_csr(state, CSR_SATP);
		int pte_addr = (satp & (((uint32_t)1 << PTE_ADDRESS_BITS) - 1)) << PAGE_SHIFT;

		int pte_bits = 12 - 2;
		int pte_mask = (1 << pte_bits) - 1;

		//printf("pte @ 0x%x\n", pte_addr);

		int levels = 2; //for sv32	
		//Letptebe the value of the PTE at address a+va.vpn[i]×PTESIZE.
		word pte;
		word paddr;

		for (int i = 0; i < levels; i++) {
			int vaddr_shift = PAGE_SHIFT + pte_bits * (levels - 1 - i);
			int pte_idx = (virtual_address >> vaddr_shift) & pte_mask;
			pte_addr += pte_idx << 2;
			pte = read_word_physical(state, pte_addr);
			//the V bit indicates whether the PTE is valid
			//If pte.v= 0, or if pte.r= 0 and pte.w= 1, stop and raise a page-fault exception.
			if (!(pte & PTE_V_MASK))
				return PAGE_FAULT; /* invalid PTE */
			//get the physical address bit
			paddr = (pte >> 10) << PAGE_SHIFT;
			word xwr = (pte >> 1) & 7;
			if (xwr != 0) {
				//writable pages must also be marked readable (no W without R)
				if (xwr == PTE_W || xwr == PTE_XW)
					return PAGE_FAULT;
				//TODO privilege check against PMP

				//when a virtual page is accessed and the A bit is clear, or is written and the D bit is clear, a page-fault exception is raised
				int accessed = pte & _PAGE_ACCESSED;
				int dirty = (access_type == STORE) * _PAGE_DIRTY;

				int accessed_or_dirty_flags = _PAGE_ACCESSED | (access_type == STORE? _PAGE_DIRTY : 0);
				if ((pte & accessed_or_dirty_flags) != accessed_or_dirty_flags) {
#ifdef ENABLE_DIRTY
					pte |= accessed_or_dirty_flags;
					//update pte 
					write_word_physical(state, pte_addr, pte);
#else
					return PAGE_FAULT;
#endif
				}

				//TODO it looks like we should be setting the accessed bit

				//and 

				word vaddr_mask = ((word)1 << vaddr_shift) - 1;
				//add the virtual address offset
				word result = (virtual_address & vaddr_mask) | (paddr & ~vaddr_mask);
				*physical_address = result;
				return TRANSLATE_OK;
			}
			else { //R is 0 or X is 0, this is a pointer to the next level
				pte_addr = paddr; //go one level down
			}
		}
		//
		//TODO refactor around this to allow for more "direct" read, or another "overload" on read_common with forced mode
		//MemoryTarget bare = get_memory_target_bare(state, pte_addr);
		//word value = read_common_ram(state, bare.ptr, SIZE_WORD);

		// When Sv32 virtual memory mode is selected in the MODE field of the satp register,
		//supervisor virtual addresses are translated into supervisor physical addresses via a two-level page table.   
		//The  20-bit  VPN  is  translated  into  a  22-bit  physical  page  number  (PPN),  
		//while  the  12-bit page offset is untranslated.  
		//The resulting supervisor-level physical addresses are then checkedusing any physical memory 
		//protection structures (Sections 3.6), before being directly converted to machine-level physical addresses.
		//printf("pte value: 0x%x", pte_value);
		//physical_address = -1; //raise page fault
	}
	else {
		printf(__FILE__ ": error: invalid SATP mode %d", memory_mode);
		exit(1);
	}
	return TRANSLATE_OK;
}

//returns 0 on success, negative value on exception
int get_memory_target(State* state, word virtual_address, enum access_type access_type, MemoryTarget* target) {
	word physical_address;
	int result = translate_address(state, virtual_address, access_type, &physical_address);
	if (result == PAGE_FAULT) {
		state->pending_exception = CAUSE_LOAD_PAGE_FAULT;
		state->pending_tval = virtual_address;
		return result;
	}
	return get_memory_target_physical(state, physical_address, target);
}

void write_common(State* state, word address, word value, int size_log2) {
	MemoryTarget target;
	if (state->pc == 0xc0000048) {
		int a = 3;
	}
	int status = get_memory_target(state, address, STORE, &target);
	if (target.range->is_ram) {
		write_common_ram(state, target.ptr, value, size_log2);
	}
	else {
		//device I/O
		target.range->write_func(target.range->opaque, target.ptr, value, size_log2);
	}
}

void write_word(State* state, word address, word value) {
	write_common(state, address, value, SIZE_WORD);
}

void write_halfword(State* state, word address, halfword value) {
	write_common(state, address, value, SIZE_HALF);
}

void write_byte(State* state, word address, byte value) {
	write_common(state, address, value, SIZE_BYTE);
}

// READ 

word read_common_ram(State* state, uint8_t* target, int size_log2) {
	word value;
	switch (size_log2) {
	case SIZE_WORD:
		value = *target
			| *(target + 1) << 8
			| *(target + 2) << 16
			| *(target + 3) << 24;
		break;
	case SIZE_HALF:
		value = *target | *(target + 1) << 8;
		break;
	case SIZE_BYTE:
		value = *target;
		break;
	default:
		fprintf(stderr, "read_common_ram unsupported size:%d", size_log2);
		exit(1);
		return 0;
	}
	return value;
}

word read_common(State* state, word address, int size_log2) {
	MemoryTarget target;
	int status = get_memory_target(state, address, LOAD, &target);
	if (target.range->is_ram) {
		return read_common_ram(state, target.ptr, size_log2);
	}
	else {
		//device I/O
		return target.range->read_func(target.range->opaque, target.ptr, size_log2);
	}
}

word read_word(State* state, word address) {
	return read_common(state, address, SIZE_WORD);
}

word read_halfword_signed(State* state, word address) {
	word value = read_common(state, address, SIZE_HALF);
	//sign extend 16-bit value
	if ((value & 0x8000) == 0x8000)
		return value | 0xffff0000;
	else
		return value;
}

word read_halfword_unsigned(State* state, word address) {
	word value = read_common(state, address, SIZE_HALF);
	return value;
}

word read_byte_signed(State* state, word address) {
	word value = read_common(state, address, SIZE_BYTE);
	//sign extend 8-bit value
	if ((value & 0x80) == 0x80)
		return value | 0xffffff00;
	else
		return value;
}

word read_byte_unsigned(State* state, word address) {
	word value = read_common(state, address, SIZE_BYTE);
	return value;
}

