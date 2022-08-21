#ifndef HAVE_UTIL_H
#define HAVE_UTIL_H
#include "common.h"
#include <stdbool.h>

/**
 * @brief Convert an unsigned long to unsigned integer safely, limit the range between 0~UINT_MAX
 * 
 * @param value The unsinged long to convert
 * @return unsigned int The converted unsinged integer
 */
unsigned int util_uint_from_ulong(unsigned long value);

/**
 * @brief Convert a long to integer safely, limit the range between INT_MIN~INT_MAX
 * 
 * @param value The long to convert
 * @return int The converted integer
 */
int util_int_from_long(long value);

#if 0
long util_file_get_length(FILE *fp);

long util_file_get_length_and_rollback(FILE *fp);

long util_file_get_length_and_restart(FILE *fp);

char *util_unescape_mountinfo(char *escaped);
#endif

/**
 * @brief Unescape oct sequence in mountinfo's root & mountpoint in place (\040 -> ' ', \011 -> '\t', \012 -> '\n', \134 -> '\\)
 * 
 * @param escaped The character array (string) to unescape
 */
void util_unesacpe_mountinfo_in_place(char *escaped);

/**
 * @brief Create a directory on demand
 * 
 * @param path The path to the folder
 * @param mode The oct mode, usually should be 0755
 * @return int 0 if not exist and create successfully, or already exists; 1 already exists and not folder, or can't check
 */
int util_mkdir(const char *path, mode_t mode);

/**
 * @brief Create a directory recursively on demand
 * 
 * @param path The path to the folder
 * @param mode The oct mode, usually should be 0755
 * @return int 0 if not exist and create successfully, or already exists; 1 already exists and not folder, or can't check
 */
int util_mkdir_recursive(const char *path, mode_t mode);
#endif