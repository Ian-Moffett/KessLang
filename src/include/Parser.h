#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "AST.h"
#include "Debugger.h"
#include "util/kl_assert.h"
#include <stdbool.h>


typedef struct {
    token_t curToken;
    unsigned long idx;
    tokenlist_t tokenlist;
    ast_t ast;
    bool error;
} parser_t;


void parse(parser_t* parser);


#endif
