#include "memory_utils.h"

void* mallocz(size_t size)
{
	void* ptr;
	ptr = malloc(size);
	if (!ptr)
		return NULL;
	memset(ptr, 0, size);
	return ptr;
}