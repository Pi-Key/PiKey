#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "pk_errors.h"

char* formatter(char* format_string, ...) {
    va_list args;
    va_start(args, format_string);

    int size = vsnprintf(NULL, 0, format_string, args);
    va_end(args);

    char* result = (char*)malloc(size + 1);
    if (result == NULL) {
        pk_error("There was a memory allocation error when trying to format a string.");
        return NULL;
    }

    va_start(args, format_string);
    vsnprintf(result, size + 1, format_string, args);
    va_end(args);

    return result;
}
