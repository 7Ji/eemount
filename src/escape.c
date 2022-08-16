#include "escape_p.h"

bool escape_character(char character){
    if (('0'<=character && character<='9') ||
        ('A'<=character && character<='Z') ||
        ('a'<=character && character<='z')) {
        return false;
    } else {
        return true;
    }
}

unsigned int* escape_string_ids(char *string, unsigned int *len_array) {
    unsigned int len, id_string, id_array;
    unsigned int *escape_array;
    char character;

    len = strlen(string);
    escape_array = calloc(len, sizeof(unsigned int));
    id_array = 0;
    for(id_string=0; id_string<len; ++id_string) {
        character = string[id_string];
        if (character == '\0') {
            break;
        }
        if (('0'<=character && character<='9') ||
            ('A'<=character && character<='Z') ||
            ('a'<=character && character<='z')) {
            continue;
        } else {
            escape_array[id_array++] = id_string;
        }
    }
    escape_array = realloc(escape_array, sizeof(unsigned int) * id_array);
    *len_array = id_array;
    return escape_array;
}

char *escape_string(char *string) {
    unsigned int len_string, id_string, len_array, id_array, id_escape;
    unsigned int *escape_array;
    char *escape_string;
    char character;
    char escape_character[4];

    len_string = strlen(string);
    /* Don't care about empty string */
    if (len_string == 0) {
        return NULL;
    }
    escape_array = escape_string_ids(string, &len_array);
    if (len_array == 0) {
        escape_string = strdup(string);
        return escape_string;
    }
    escape_string = calloc(len_string + len_array*2 + 1, sizeof(char));
    id_array = 0;
    id_escape = escape_array[id_array];
    for (id_string = 0; id_string < len_string; ++id_string) {
        character = string[id_string];
        if (id_string != id_escape) {
            escape_string[id_string + id_array*2] = character;
        } else {
            sprintf(escape_character, "_%02x", character);
            strcat(escape_string, escape_character);
            ++id_array;
            if (id_array < len_array) {
                id_escape = escape_array[id_array];
            }
        }
    }
    return escape_string;
}