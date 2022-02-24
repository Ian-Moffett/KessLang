#include "include/Parser.h"

static inline token_t peek(parser_t* parser, unsigned long idx) {
    return parser->tokenlist.tokens[idx];
}


static inline void parser_advance(parser_t* parser) {
    ++parser->idx;
    parser->curToken = parser->tokenlist.tokens[parser->idx];
}


bool inFunction = false;

static inline void usescope(ast_node_t* node, scope_t scope) {
    if (scope == LOCAL) {
        node_push_child(node, createChild("SCOPE", "LOCAL", false));
        inFunction ? node_push_child(node, createChild("INFUNC", "YES", false)) : node_push_child(node, createChild("INFUNC", "NO", false));
    } else {
        node_push_child(node, createChild("SCOPE", "GLOBAL", false));
        inFunction ? node_push_child(node, createChild("INFUNC", "YES", false)) : node_push_child(node, createChild("INFUNC", "NO", false));
    }
}


inline void parse(parser_t* parser) { 
    ast_t kl_ast = {
        .size = 0,
        .nodes = (ast_node_t*)malloc(sizeof(ast_node_t)),
    };

    parser->ast = kl_ast;

    unsigned long line = 1;
    scope_t curScope = GLOBAL;

    while (parser->idx < parser->tokenlist.size && !(parser->error)) {
        parser->curToken = parser->tokenlist.tokens[parser->idx];

        switch (parser->curToken.type) {
            case T_FUNCTION_CALL:
                ast_push_node(&parser->ast, createNode("CALL", parser->curToken.tok, false, line));
                break;
            case T_STR:
                ast_push_node(&parser->ast, createNode("STR", parser->curToken.tok, false, line));
                break;
            case T_FUNC:
                if (curScope == LOCAL) {
                    parser->error = true;
                    kl_log_err("ScopeError: Already in local scope.", "", line);
                    break;
                }

                curScope = LOCAL;
                parser_advance(parser); 

                // TODO: Add regular functions (non asm embeded functions).
                kl_assert(parser->curToken.type == T_ASM_MACRO, "UnsupportedError: We will add regular functions soon! For now use __asm macro.", parser, PARSER_STAGE, line, "");
                if (parser->error) {
                    break;
                }


                parser_advance(parser);

                ast_node_t funcNode = createNode("FUNC", parser->curToken.tok, false, line);
                node_push_child(&funcNode, createChild("ASM", "YES", false));

                parser_advance(parser);
                kl_assert(parser->curToken.type == T_LPAREN, "SyntaxError: Expected '(' after function identifier.", parser, PARSER_STAGE, line, "");
                ast_push_node(&parser->ast, funcNode);

                if (parser->error) {
                    break;
                }

                // TODO: When allowing regular functions, change this.
                kl_assert(parser->curToken.type != T_VAR_PREFIX, "SyntaxError: No arguments allowed for function with __asm marcro (yet).", parser, PARSER_STAGE, line, "");

                if (parser->error) {
                    break;
                }

                parser_advance(parser);
                kl_assert(parser->curToken.type == T_RPAREN, "SyntaxError: Expected ')'.", parser, PARSER_STAGE, line, "");
                parser_advance(parser);
                kl_assert(parser->curToken.type == T_LBRACE, "SyntaxError: Expected '{'.", parser, PARSER_STAGE, line, "");
                break;
            case T_LBRACE:    // No reason to have a stray brace.
                parser->error = true;
                kl_log_err("SyntaxError: Unexpected token found.", "{", line);
                break;
            case T_RBRACE:
                if (curScope != LOCAL) {
                    parser->error = true;
                    kl_log_err("SyntaxError: Unexpected token found.", "}", line);
                } else {
                    ast_push_node(&parser->ast, createNode("FLAG", "__GLB_SCOPE_START", false, line));
                    curScope = GLOBAL;
                }

                break;
            case T_PRINT:
                if (peek(parser, parser->idx + 1).type != T_VAR_PREFIX && peek(parser, parser->idx + 1).type != T_EOL) {
                    ast_push_node(&parser->ast, createNode("PRINT", peek(parser, parser->idx + 1).tok, false, line));
                } else if (peek(parser, parser->idx + 1).type == T_VAR_PREFIX) {
                    unsigned char attr = 0x0;
                    const unsigned char ATTR_INDEXOF = 0x1;

                    ast_node_t printNode;

                    if (peek(parser, parser->idx + 3).type == T_LSQR_BRACKET) {
                        printNode = createNode("VAR_PRINT", peek(parser, parser->idx + 2).tok, false, line);
                        kl_assert(peek(parser, parser->idx + 4).type == T_INT, "SyntaxError: Expected integer after '['", parser, PARSER_STAGE, line, "");

                        if (parser->error) {
                            break;
                        }

                        kl_assert(peek(parser, parser->idx + 5).type == T_RSQR_BRACKET, "SyntaxError: Expected ']'.", parser, PARSER_STAGE, line, "");

                        if (parser->error) {
                            break;
                        }

                        attr |= ATTR_INDEXOF;
                    } else {
                        printNode = createNode("VAR_PRINT", peek(parser, parser->idx + 2).tok, false, line);
                    }

                    if (!(attr & 0xFF)) {
                        node_push_child(&printNode, createChild("ATTR", "NONE", false));
                    } else if (attr & ATTR_INDEXOF) {
                        node_push_child(&printNode, createChild("ATTR", "INDEXOF", false));
                        node_push_child(&printNode, createChild("INDEXOF", peek(parser, parser->idx + 4).tok, false));
                    }

                    ast_push_node(&parser->ast, printNode);
                }

                break;
            case T_VAR_PREFIX:
                ast_node_t varNode;

                {
                    unsigned char attr = 0x0;
                    const unsigned char ATTR_CONST = 0x1;

                    if (peek(parser, parser->idx + 2).type == T_EQUALS_OP) {
                        if (peek(parser, parser->idx + 1).tok[0] == '&') {
                            attr |= ATTR_CONST;

                            const int symbolLen = strlen(peek(parser, parser->idx + 1).tok);
                            char symbolCpy[symbolLen];
                            strcpy(symbolCpy, peek(parser, parser->idx + 1).tok);

                            unsigned int heapAllocSymbIdx = 0;

                            for (int i = 1; i < strlen(symbolCpy); ++i, ++heapAllocSymbIdx) {
                                peek(parser, parser->idx + 1).tok[heapAllocSymbIdx] = symbolCpy[i];
                            }

                            peek(parser, parser->idx + 1).tok[heapAllocSymbIdx] = '\0';
                        } 
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
                    
                    if (!(attr & 0xFF)) {
                        node_push_child(&varNode, createChild("ATTR", "NONE", false));
                    } else if (attr & ATTR_CONST) {
                        node_push_child(&varNode, createChild("ATTR", "CONST", false));
                    }

                    usescope(&varNode, curScope);
                    ast_push_node(&parser->ast, varNode);
                }

                break;
            case T_DEREF_OP:
                if (peek(parser, parser->idx + 1).type == T_INT && peek(parser, parser->idx + 2).type == T_EQUALS_OP) {
                    kl_assert(strlen(peek(parser, parser->idx + 3).tok) >= 1, "SyntaxError: Expected value after assignment operator.", parser, PARSER_STAGE, line, "");
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


                    usescope(&derefNode, curScope);
                    ast_push_node(&parser->ast, derefNode);

                } else if (peek(parser, parser->idx + 1).type == T_VAR_PREFIX && peek(parser, parser->idx + 3).type == T_EQUALS_OP && peek(parser, parser->idx + 4).type != T_VAR_PREFIX) {
                    kl_assert(strlen(peek(parser, parser->idx + 4).tok) >= 1, "SyntaxError: Expected value after assignment operator.", parser, PARSER_STAGE, line, "");
                    ast_node_t derefNode = createNode("DEREF_VAR", peek(parser, parser->idx + 2).tok, false, line);
                    
                    if (peek(parser, parser->idx + 4).type == T_INT) {
                        node_push_child(&derefNode, createChild("INT", peek(parser, parser->idx + 4).tok, false));
                    } else if (peek(parser, parser->idx + 4).type == T_STR) {                    
                        node_push_child(&derefNode, createChild("STR", peek(parser, parser->idx + 4).tok, false));
                    }


                    usescope(&derefNode, curScope);
                    ast_push_node(&parser->ast, derefNode);
                }

                ++parser->idx;
                ++parser->idx;
                break;
            case T_EOL:
                ++line;
                break;
        }

        ++parser->idx;
    }
}
