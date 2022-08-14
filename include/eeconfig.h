#include "common.h"

#include <stdbool.h>

bool eeconfig_initialize();
void eeconfig_close();
char* eeconfig_get_string(const char *key);
int eeconfig_get_int(const char *key);
bool eeconfig_get_bool(const char *key, const bool bool_default);