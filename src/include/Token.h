#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include <stdbool.h>


typedef enum {
    T_PRINT,
    T_STR,
    T_INT,
    T_IDENTIFIER,
    T_EQUALS_OP,
    T_VAR_PREFIX,
    T_EOL,
    T_END_STATEMENT,
} tokentype_t;


typedef struct {
    tokentype_t type;
    char* tok;
    bool alloc;
} token_t;


typedef struct {
    token_t* tokens;
    size_t size;
} tokenlist_t;


void destroy_tokenlist(tokenlist_t* tokenlist);
void push_token(tokenlist_t* tokenlist, token_t token);
token_t create_token(tokentype_t type, char* tok, bool alloc);


#endif
