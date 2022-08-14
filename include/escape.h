#include "common.h"
#include <stdbool.h>
#include <string.h>

/**
Try to escape character

 \param character the character to escape
 \returns false if no need to escape; true if need to escape
*/
bool escape_character(char character);

/**
Get the id of characters in a string that should be escaped
 \param string the pointer to the string
 \param len_array the pointer to an unsigned int to return the length of array

 \returns an array of the id of characters that should be escaped
*/
unsigned int* escape_string_ids(char *string, unsigned int *len_array);

/**
Get the escaped string from a raw string
 \param string the raw string
 
 \returns a pointer to the escaped string, NULL if the raw string is empty
*/
char *escape_string(char *string);