#ifndef VAR_H
#define VAR_H

#include <stddef.h>

typedef struct {
    char* key;
    bool isNumeric;
    char* strVal;
    ssize_t numericVal;
    bool used;
    bool constant;
    int count;
} var_t;


typedef struct {
    bool assembly;
    bool used;
} function_t;



#endif
