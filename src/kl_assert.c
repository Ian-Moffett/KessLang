#include "include/util/kl_assert.h"



void kl_assert(unsigned char assertion, const char* fail_msg, void* stage_struct, stage_type_t stage_type, int lineNum, char* offendingToken) {
    if (!(assertion)) {
        switch (stage_type) {
            case LEXER_STAGE: 
            {
                kl_log_err(fail_msg, offendingToken, lineNum);
                lexer_t* lexer = (lexer_t*)stage_struct;
                lexer->error = true;
            }

            break;
            case PARSER_STAGE: 
            {

                kl_log_err(fail_msg, offendingToken, lineNum);
                parser_t* parser = (parser_t*)stage_struct;
                parser->error = true;
            }

            break;
        }
    }
}
