#include "util_p.h"

unsigned int util_uint_from_ulong(unsigned long value) {
    if (value > UINT_MAX) {
        logging(LOGGING_WARNING, "Limiting unsigned long %lu to %u as it is too big", value, UINT_MAX);
        return UINT_MAX;
    }
    return value;
}

int util_int_from_long(long value) {
    if (value > INT_MAX) {
        logging(LOGGING_WARNING, "Limiting long %ld to %d as it is too big", value, INT_MAX);
        return INT_MAX;
    } 
    if (value < INT_MIN) {
        logging(LOGGING_WARNING, "Limiting long %ld to %d as it is too small", value, INT_MIN);
        return INT_MIN;
    }
    return value;
}

long util_file_get_length(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

long util_file_get_length_and_rollback(FILE *fp) {
    long mark = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, mark, SEEK_SET);
    return length;
}

long util_file_get_length_and_restart(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return length;
}