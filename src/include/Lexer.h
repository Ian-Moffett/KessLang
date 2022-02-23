#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include "Debugger.h"
#include "config/Reserved.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>


typedef struct {
    unsigned long long idx;
    char curChar;
    tokenlist_t tokenlist;
    unsigned long long lineNum;
    bool error;
} lexer_t;

void tokenize(lexer_t* lexer, char* buffer);

#endif
