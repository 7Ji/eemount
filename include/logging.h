#ifndef HAVE_LOGGING_H
#define HAVE_LOGGING_H
#include "common.h"

/**
 * @brief Enumerate of all possible logging levels
 * 
 */
enum logging_levels {
    /**
     * @brief Most detailed logging. The result should not be shown to user except for development
     * 
     */
    LOGGING_DEBUG,
    /**
     * @brief Normal logging. This is the log we want users to read
     * 
     */
    LOGGING_INFO,
    /**
     * @brief Something wrong happened but this is still recoverable. Notify the user nontheless
     * 
     */
    LOGGING_WARNING,
    /**
     * @brief Something very wrong happened, and the current routine is broken. Notify the user nontheless
     * 
     */
    LOGGING_ERROR,
    /**
     * @brief Something extremely wrong happened, the wrong program is broken. Notify the user nontheless. This should only be used in main routine
     * 
     */
    LOGGING_FATAL
};

/**
 * @brief Logging something. Automatically append new line
 * 
 * @param level The logging level: DEBUG, INFO, WARNING, ERROR, FATAL
 * @param format The format string
 * @param ... The format variables
 */
void logging(const int level, const char *format, ...);

// /**
//  * @brief Get the logging file descriptor, if you want to send something to it directly
//  * 
//  * @param level The logging level: DEBUG, INFO, WARNING, ERROR, FATAL
//  * @return FILE* The corresponding file descriptor, usually stdout or stderr
//  */
// FILE *logging_get_target(const int level);
#endif