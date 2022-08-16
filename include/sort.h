#ifndef HAVE_SORT_H
#define HAVE_SORT_H
#include "common.h"

/**
 * @brief Compare two strings
 * 
 * @param a One of the string to compare
 * @param b The other string to sort
 * @return int >0 if a is greater, <0 if b is greater, 0 if equal
 */
int sort_compare_string(const void *a, const void *b);

/**
 * @brief Compare two drive struct
 * 
 * @param a One of the drive struct to compare 
 * @param b The other drive struct to compare
 * @return int >0 if a's name is greater, <0 if b's name is greater, 0 if equal
 */
int sort_compare_drive(const void *a, const void *b);

/**
 * @brief Compare two systemd_mount struct
 * 
 * @param a One of the systemd_mount struct to compare
 * @param b The other systemd_mount struct to compare
 * @return int >0 if a's name is greater, <0 if b's name is greater, 0 if equal
 */
int sort_compare_systemd_mount(const void *a, const void *b);
#endif