#include "sort.h"
#include "string.h"
#include "drive.h"

int sort_compare_string(const void *a, const void *b){
    return strcmp(*(char**)a, *(char**)b);
}

int sort_compare_drive(const void *a, const void *b) {
    return strcmp(((struct drive *)a)->name, ((struct drive *)b)->name);
}