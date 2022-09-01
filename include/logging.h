/*
 eemount (Multi-source ROMs mounting utility for EmuELEC) is licensed under [**GPL3**](https://gnu.org/licenses/gpl.html)

 Copyright (C) 2022 Guoxin "7Ji" Pu (pugokushin@gmail.com)

 This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version * of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

*/
#ifndef HAVE_LOGGING_H
#define HAVE_LOGGING_H
#include "common.h"

/**
 * @brief Enumerate of all possible logging levels
 * 
 */
enum logging_levels {
    /**
     * @brief disable logging, this should not be used, and only used internally
     * 
     */
    LOGGING_DISABLED,
    /**
     * @brief Something extremely wrong happened, the wrong program is broken. Notify the user nontheless. This should only be used in main routine
     * 
     */
    LOGGING_FATAL,
    /**
     * @brief Something very wrong happened, and the current routine is broken. Notify the user nontheless
     * 
     */
    LOGGING_ERROR,
    /**
     * @brief Something wrong happened but this is still recoverable. Notify the user nontheless
     * 
     */
    LOGGING_WARNING,
    /**
     * @brief Normal logging. This is the log we want users to read
     * 
     */
    LOGGING_INFO,
    /**
     * @brief Most detailed logging. The result should not be shown to user except for development
     * 
     */
    LOGGING_DEBUG,
};

/**
 * @brief Initialize the logging subsystem, most importantly, init the timer to 0
 * 
 */
void logging_initialize();

/**
 * @brief Logging something. Automatically append new line
 * 
 * @param level The logging level: DEBUG, INFO, WARNING, ERROR, FATAL
 * @param format The format string
 * @param ... The format variables
 * @return 0 logging output success, 1 logging omitted due to logging level (e.g. trying to log to DEBUG when logging level is at ERROR)
 */
int logging(const enum logging_levels level, const char *format, ...);

/**
 * @brief Setting the logging level
 * 
 * @param level The LOGGING_LEVEL to set
 * @return 0 level updated, 1 level invalid
 */
int logging_set_level(int level);
#endif