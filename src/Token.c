#include "include/Token.h"
#include <stdio.h>


void destroy_tokenlist(tokenlist_t* tokenlist) {
    for (int i = 0; i < tokenlist->size; ++i) {
        if (tokenlist->tokens[i].alloc) { 
            free(tokenlist->tokens[i].tok);
        }
    }

    free(tokenlist->tokens);
    tokenlist->tokens = NULL;
}


void push_token(tokenlist_t* tokenlist, token_t token) {
    tokenlist->tokens[tokenlist->size] = token;
    ++tokenlist->size;
    tokenlist->tokens = (token_t*)realloc(tokenlist->tokens, sizeof(token_t) * (tokenlist->size + 2));    
}



token_t create_token(tokentype_t type, char* tok, bool alloc) {
    token_t newTok = {
        .type = type,
        .tok = tok,
        .alloc = alloc
    };

    return newTok;
}
