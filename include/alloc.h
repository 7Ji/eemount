#ifndef HAVE_ALLOC_H
#define HAVE_ALLOC_H

#include "common.h"

#define ALLOC_BASE_SIZE     16
#define ALLOC_MULTIPLIER   1.5

/**
 * @brief (Re)allocate memory according to given for pointer
 * 
 * @param ptr pointer to the original pointer which could have allocated memory, if it's a pointer, use realloc, else, use malloc
 * @param size the new size of the memory
 * @return void* the new pointer if allocate success, or NULL if failed
 */
void *alloc_optional_resize(void *ptr, size_t size);

/**
 * @brief Free the memory and set pointer to NULL if it's used
 * 
 * @param ptr pointer to free
 */

void alloc_free_if_used(void **ptr);
#endif