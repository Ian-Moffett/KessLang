#ifndef KL_ASSERT_H
#define KL_ASSERT_H

#include "../Lexer.h"
#include "../Parser.h"
#include "../Debugger.h"
#include <stdio.h>

typedef enum {
    LEXER_STAGE,
    PARSER_STAGE,
} stage_type_t;


void kl_assert(unsigned char assertion, const char* fail_msg, void* stage_struct, stage_type_t stage_type, int lineNum, char* offendingToken);

#endif
