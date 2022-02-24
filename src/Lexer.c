#include "include/Lexer.h"


static bool kl_lex_isint(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return true;
    }

    return false;
}


static bool kl_lex_ishex(char c) {
    switch (c) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'A':
        case 'a':
        case 'B':
        case 'b':
        case 'C':
        case 'c':
        case 'D':
        case 'd':
        case 'E':
        case 'e':
        case 'F':
        case 'f':
            return true;

    }

    return false;
}


static char* kl_lex_get_int(char* buffer, lexer_t* lexer) {
    char* intBuf = (char*)calloc(2, sizeof(char));
    unsigned ibidx = 0;

    while (kl_lex_isint(lexer->curChar)) {
        lexer->curChar = buffer[lexer->idx];

        if (!(kl_lex_isint(lexer->curChar))) {
            bool end = false;
            switch (lexer->curChar) {
                case '=':
                case '\n':
                case '\0':
                case ';':
                    end = true;
                    break;
            }

            if (end) {
                break;
            } else {
                intBuf[ibidx] = lexer->curChar;
                ++ibidx;
                lexer->error = true;
                kl_log_err("TokenError: Invalid token found while lexing.", intBuf, lexer->lineNum);
            }
        }

        intBuf[ibidx] = lexer->curChar;
        ++ibidx;
        intBuf = (char*)realloc(intBuf, sizeof(char) * (ibidx + 2));

        ++lexer->idx;
    }

    return intBuf;
}


static char* kl_lex_get_func(char* buffer, lexer_t* lexer) {
    char* fbuf = (char*)calloc(2, sizeof(char));
    unsigned int fbidx = 0;

    while (lexer->curChar == ' ' || lexer->curChar == '\t') {         // Skip whitespace.
        ++lexer->idx;
        lexer->curChar = buffer[lexer->idx];
    }

    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        if (!(isalpha(lexer->curChar)) && lexer->curChar != '_') { 
            break;
        }

        fbuf[fbidx] = lexer->curChar;
        ++fbidx;
        fbuf = (char*)realloc(fbuf, sizeof(char) * (fbidx + 2));
        ++lexer->idx;
    }

    return fbuf;
}


static char* kl_lex_get_hex(char* buffer, lexer_t* lexer) {
    char* hexBuf = (char*)calloc(2, sizeof(char));
    unsigned int hidx = 0;

    bool start = true;

    while (lexer->idx < strlen(buffer)) {
        lexer->curChar = buffer[lexer->idx];

        
        if (lexer->curChar == 'x' || lexer->curChar == 'X' && start) {
            hexBuf[hidx] = lexer->curChar;
            ++hidx;
            hexBuf = (char*)realloc(hexBuf, sizeof(char) * (hidx + 2));
            ++lexer->idx;
            start = false;
            continue;
        }

        if (!(kl_lex_ishex(lexer->curChar))) {
            ++lexer->idx;
            lexer->curChar = buffer[lexer->idx];
            break;
        }

        hexBuf[hidx] = lexer->curChar;
        ++hidx;
        hexBuf = (char*)realloc(hexBuf, sizeof(char) * (hidx + 2));
        ++lexer->idx;
    }

    return hexBuf;
}


static char* kl_lex_get_var(char* buffer, lexer_t* lexer) {
    char* varBuf = (char*)calloc(2, sizeof(char));
    unsigned int vbidx = 0;
    bool run = true;

    while (lexer->idx < strlen(buffer) && run) {
        ++lexer->idx;
        lexer->curChar = buffer[lexer->idx];

        switch (lexer->curChar) {
            case ';':
                --lexer->idx;
                lexer->curChar = ';';
            case ' ':
            case '\t':
            case '=':
                run = false;
                continue;
        }

        varBuf[vbidx] = lexer->curChar;
        ++vbidx;
        varBuf = (char*)realloc(varBuf, sizeof(char) * (vbidx + 2));
    }

    return varBuf;
}


static char* kl_lex_get_str(char* buffer, lexer_t* lexer) {
    char* stringBuf = (char*)calloc(2, sizeof(char));
    unsigned long strBufIdx = 0;

    while (lexer->idx < strlen(buffer)) {
        ++lexer->idx;
        lexer->curChar = buffer[lexer->idx];

        if (lexer->curChar == '"') {
            break;
        } else if (lexer->curChar == ';' || lexer->curChar == '\n') {
            kl_log_err("SyntaxError: EOL found before '\"'.", "", lexer->lineNum);
            lexer->error = true;
            free(stringBuf);
            return NULL;
        }

        stringBuf[strBufIdx] = lexer->curChar; 
        ++strBufIdx;
        stringBuf = (char*)realloc(stringBuf, sizeof(char) * (strBufIdx + 2));
    }

    ++lexer->idx;
    return stringBuf;
}


void tokenize(lexer_t* lexer, char* buffer) {
    char* lexBuf = (char*)calloc(2, sizeof(char));
    unsigned int lbidx = 0;

    bool run = true;
    bool ignoreErrors = false;
    bool skipSpaces = false;
    bool nobufadd = false;

    bool semicolonFound = false;

    while (lexer->idx < strlen(buffer) && run && !(lexer->error)) {
        lexer->curChar = buffer[lexer->idx]; 
        const char C_SYM_ARR[2] = COMMENT_SYM;    // Convert comment symbol (index 0) to string.
        char COMMENT_SYM_0[2] = {'\0'};           // Null terminate.
        COMMENT_SYM_0[0] = C_SYM_ARR[0];          // Set the value.

        if (lexer->curChar == '0' && buffer[lexer->idx + 1] == 'x') {
            /*
             *
             *  If there is a value prefixed with '0x' then 
             *  the lexer will assume it is a hex digit.
             *  and start grabbing the rest of the digits.
             */
            char* hex = kl_lex_get_hex(buffer, lexer);      // Grab the hex digit.
            push_token(&lexer->tokenlist, create_token(T_INT, hex, true));          // Push hex digit as INT token.
            skipSpaces = true;                                                      // Skip spaces so we don't get errors.
            ignoreErrors = true;

            // Reset buffer.
            lbidx = 0;  
            lexBuf = (char*)realloc(lexBuf, sizeof(char));

        }

        if (kl_lex_isint(lexer->curChar)) {            
            char* dec = kl_lex_get_int(buffer, lexer);
            // TODO: Add support for ADD, SUB, DIV, and MUL.
            push_token(&lexer->tokenlist, create_token(T_INT, dec, true));

            if (lexer->error) {
                break;
            }

            skipSpaces = true;
            ignoreErrors = true;
            lbidx = 0;
            lexBuf = (char*)realloc(lexBuf, sizeof(char));
        }


        if (lexer->curChar == VAR_PREFIX[0]) {          // If we see a var prefix, start looking for vars.
            char var_prefix[2] = "\0\0";
            var_prefix[0] = VAR_PREFIX[0];

            push_token(&lexer->tokenlist, create_token(T_VAR_PREFIX, var_prefix, false));         // Push var prefix as token.
            char* symbol = kl_lex_get_var(buffer, lexer);
            // Check variable naming.
            if (symbol[0] < 'a' && symbol[0] > 'z') {                                   
                if (symbol[0] != '_' && symbol[0] < 'A' && symbol[0] > 'Z') {
                    kl_log_err("SyntaxError: Invalid naming for variable.", "", lexer->lineNum);
                    lexer->error = true;
                    break;
                }
            }

            for (int i = 0; i < strlen(symbol); ++i) {
                // Check variable naming.
                if (symbol[i] < 'a' && symbol[i] > 'z') {
                    if (symbol[i] != '_' && symbol[i] < 'A' && symbol[i] > 'Z') {
                        kl_log_err("SyntaxError: Invalid character for variable found.", "", lexer->lineNum);
                        lexer->error = true;
                        break;
                    }
                } 
            }

            if (lexer->error) {
                continue;
            }

            // Push symbol and reset everything.
            push_token(&lexer->tokenlist, create_token(T_IDENTIFIER, symbol, true));
            ignoreErrors = true;
            skipSpaces = true;
            ++lexer->idx;
            lbidx = 0;
            lexBuf = (char*)realloc(lexBuf, sizeof(char));
            continue;
        } else if (lexer->curChar == CALL_PREFIX[0]) { 
            if (buffer[lexer->idx + 1] == CALL_PREFIX[1]) {
                lexer->idx += 2;
                lexer->curChar = buffer[lexer->idx];
                char* identifier = kl_lex_get_func(buffer, lexer);
                push_token(&lexer->tokenlist, create_token(T_FUNCTION_CALL, identifier, true));
            }
        }

        switch (lexer->curChar) {
            case ',':
                push_token(&lexer->tokenlist, create_token(T_COMMA, ",", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case '{':
                push_token(&lexer->tokenlist, create_token(T_LBRACE, "{", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case '}':
                push_token(&lexer->tokenlist, create_token(T_RBRACE, "}", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case '(':
                push_token(&lexer->tokenlist, create_token(T_LPAREN, "(", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case ')':
                push_token(&lexer->tokenlist, create_token(T_RPAREN, ")", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case '*':
                // If we get a '*' we will assume it is a deref operator.
                push_token(&lexer->tokenlist, create_token(T_DEREF_OP, "*", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                continue;
            case '=':
                push_token(&lexer->tokenlist, create_token(T_EQUALS_OP, "=", false));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                ++lexer->idx;
                skipSpaces = true;
                break;
            case '"':       // Get string.
                char* str = kl_lex_get_str(buffer, lexer);
                push_token(&lexer->tokenlist, create_token(T_STR, str, true));
                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                skipSpaces = true;
                continue;
            case '\n':
                ++lexer->lineNum;
                push_token(&lexer->tokenlist, create_token(T_EOL, "", false));
            case ';':
                push_token(&lexer->tokenlist, create_token(T_END_STATEMENT, ";", false));
            case ' ':
                {
                    bool invalidTok = true;     // Will be false if token is found.

                    // Token adding & checking.

                    if (strcmp(lexBuf, IO_PRINT_STATEMENT) == 0) { 
                        push_token(&lexer->tokenlist, create_token(T_PRINT, IO_PRINT_STATEMENT, false));
                        invalidTok = false;
                    } else if (strcmp(lexBuf, FUNC_DEF) == 0) {
                        push_token(&lexer->tokenlist, create_token(T_FUNC, FUNC_DEF, false));
                        invalidTok = false; 

                        char* key = kl_lex_get_func(buffer, lexer);
    
                        if (strcmp(key, ASM_MACRO) == 0) {
                            push_token(&lexer->tokenlist, create_token(T_ASM_MACRO, ASM_MACRO, false));
                            free(key);
                            key = kl_lex_get_func(buffer, lexer);
                            push_token(&lexer->tokenlist, create_token(T_IDENTIFIER, key, true));

                            lbidx = 0;
                            memset(lexBuf, '\0', strlen(lexBuf));
                            lexBuf = (char*)realloc(lexBuf, sizeof(char));
                            continue;
                        } else {
                            push_token(&lexer->tokenlist, create_token(T_IDENTIFIER, key, true));

                            lbidx = 0;
                            memset(lexBuf, '\0', strlen(lexBuf));
                            lexBuf = (char*)realloc(lexBuf, sizeof(char));
                            ++lexer->idx;
                            continue;
                        }
                    } else {
                        bool nonSpace = false;
                        for (int i = 0; i < strlen(lexBuf); ++i) {
                            if (lexBuf[i] != ' ') {
                                nonSpace = true;
                            } else if (lexBuf[i] == '(' && lexBuf[i + 1] == ')') {
                                break;
                            }
                        }

                        if (!(nonSpace)) {
                            invalidTok = false;
                        }
                    }

                    if (invalidTok && !(ignoreErrors)) {
                        lexer->error = true;
                        run = false;
                        kl_log_err("TokenError: Invalid token found while lexing.", lexBuf, lexer->lineNum);
                    } else if (ignoreErrors) {
                        ignoreErrors = false;
                    }
                    
                    if (skipSpaces) {
                        while (lexer->idx < strlen(buffer) && lexer->curChar == ' ') {
                            ++lexer->idx;
                            lexer->curChar = buffer[lexer->idx];
                        }
                        
                        // ++lexer->idx;
                        skipSpaces = false;
                    }
                }

                lbidx = 0;
                lexBuf = (char*)realloc(lexBuf, sizeof(char));
                memset(lexBuf, '\0', strlen(lexBuf));
                ++lexer->idx;
                continue;
            default:
                if (lexer->curChar == COMMENT_SYM[0]) { 
                    lexer->curChar = buffer[lexer->idx];
                    if (lexer->curChar == COMMENT_SYM[1]) {
                        while (lexer->curChar != '\n') {
                            ++lexer->idx;
                            lexer->curChar = buffer[lexer->idx];
                        }

                        continue;
                    } else {
                        run = false;
                        lexer->error = true;
                        printf("%c\n", buffer[lexer->idx + 1]);
                        kl_log_err("TokenError: Invalid token found while lexing.", COMMENT_SYM_0, lexer->lineNum);
                        printf("\n\033[35mDid you mean \"%c%c\"?\n", COMMENT_SYM[0], COMMENT_SYM[1]);
                    }
                }

                break;
        }

        if (!(nobufadd)) {
            lexBuf[lbidx] = lexer->curChar;
            ++lbidx;
            lexBuf = (char*)realloc(lexBuf, sizeof(char) * (lbidx + 2));
        } else {
            nobufadd = false;
        }

        ++lexer->idx;
    }

    free(lexBuf);
}
