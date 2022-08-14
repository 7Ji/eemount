#include "common.h"

/**
 * @brief (Re)allocate memory according to size 
 * 
 * @param ptr pointer to the original pointer which could have allocated memory, if it's a pointer, use realloc, else, use malloc
 * @param size the new size of the memory
 * @return void* the new pointer if allocate success, or NULL if failed
 */
void *alloc_optional_resize(void *ptr, size_t size);

void alloc_free_if_used(void **ptr);