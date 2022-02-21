#include "include/Parser.h"

static token_t peek(parser_t* parser, unsigned long idx) {
    return parser->tokenlist.tokens[idx];
}


void parse(parser_t* parser) { 
    ast_t kl_ast = {
        .size = 0,
        .nodes = (ast_node_t*)malloc(sizeof(ast_node_t)),
    };

    parser->ast = kl_ast;

    unsigned long line = 1;

    while (parser->idx < parser->tokenlist.size && !(parser->error)) {
        parser->curToken = parser->tokenlist.tokens[parser->idx];

        switch (parser->curToken.type) {
            case T_PRINT:
                if (peek(parser, parser->idx + 1).type != T_VAR_PREFIX && peek(parser, parser->idx + 1).type != T_EOL) {
                    ast_push_node(&parser->ast, createNode("PRINT", peek(parser, parser->idx + 1).tok, false, line));
                } else if (peek(parser, parser->idx + 1).type == T_VAR_PREFIX) {
                    ast_push_node(&parser->ast, createNode("VAR_PRINT", peek(parser, parser->idx + 2).tok, false, line));
                }

                break;
            case T_VAR_PREFIX:
                ast_node_t varNode;

                {
                    if (peek(parser, parser->idx + 2).type == T_EQUALS_OP) {
                        varNode = createNode("VAR", peek(parser, parser->idx + 1).tok, false, line);
                    } else {
                        break;
                    }

                    if (peek(parser, parser->idx + 2).type == T_EQUALS_OP) {
                        int nextTok = peek(parser, parser->idx + 3).type;
                        kl_assert(nextTok == T_STR || nextTok == T_INT, "SyntaxError: Expected value after variable assignment operator.", parser, PARSER_STAGE, line, "");
                        if (parser->error) {
                            break;
                        }
                    }

                    switch (peek(parser, parser->idx + 3).type) {
                        case T_INT:
                            node_push_child(&varNode, createChild("INT", peek(parser, parser->idx + 3).tok, false));
                            break;
                        case T_STR:
                            node_push_child(&varNode, createChild("STR", peek(parser, parser->idx + 3).tok, false));
                            break;
                    }

                    ast_push_node(&parser->ast, varNode);
                }

                break;
            case T_DEREF_OP:
                if (peek(parser, parser->idx + 1).type == T_INT && peek(parser, parser->idx + 2).type == T_EQUALS_OP) {
                    kl_assert(strlen(peek(parser, parser->idx + 3).tok) == 1, "SyntaxError: Expected value after assignment operator.", parser, PARSER_STAGE, line, "");
                    ast_node_t derefNode = createNode("DEREF", peek(parser, parser->idx + 1).tok, false, line);
                    switch (peek(parser, parser->idx + 3).type) {
                        case T_INT:
                            node_push_child(&derefNode, createChild("INT", peek(parser, parser->idx + 3).tok, false));
                            break;
                        case T_STR:
                            node_push_child(&derefNode, createChild("STR", peek(parser, parser->idx + 3).tok, false));  
                            kl_assert(strlen(peek(parser, parser->idx + 3).tok) == 1, "SyntaxError: Expected char.", parser, PARSER_STAGE, line, peek(parser, parser->idx + 3).tok);
                            break;
                    }

                    ast_push_node(&parser->ast, derefNode);

                }
                break;
            case T_EOL:
                ++line;
                break;
        }

        ++parser->idx;
    }
}
