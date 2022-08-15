#ifndef HAVE_SORT_H
#define HAVE_SORT_H
#include "common.h"


int sort_compare_string(const void *a, const void *b);

int sort_compare_drive(const void *a, const void *b);

int sort_compare_systemd_mount(const void *a, const void *b);
#endif