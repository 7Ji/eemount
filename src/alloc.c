#include "alloc_p.h"

void *alloc_optional_resize(void *ptr, size_t size) {
    void *buffer;
    if (ptr) {
        buffer = realloc(ptr, size);
    } else {
        buffer = malloc(size);
    }
    if (buffer == NULL) {
        logging(LOGGING_ERROR, "Failed to allocate memory");
        if (ptr) {
            free(ptr);
        }
    }
    return buffer;
}

void alloc_free_if_used(void **ptr) {
    if (*ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}