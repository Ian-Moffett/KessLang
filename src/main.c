#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "include/Lexer.h"
#include "include/Token.h"
#include "include/Parser.h"
#include "include/AST.h"
#include "include/CodeGen.h"

bool codegenError = false;
char* org = NULL;
bool noexit = false;
bool endhalt = false;
unsigned char bits;

int main(int argc, char* argv[]) {
    // Existing file check.
    if (access("/tmp/__KL_SOURCE.S", F_OK) == 0) {
        remove("/tmp/__KL_SOURCE.S");
    }

    if (access("/tmp/__KL_OBJ.o", F_OK) == 0) {
        remove("/tmp/__KL_OBJ.o");
    } 

    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        exit(0);
    }

    bool pureAsm = false;
    bool outputObj = false;
    char* filename = NULL;
    char* outputFilename = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-s") == 0) {
            pureAsm = true;
        } else if (strcmp(argv[i], "-c") == 0) {
            outputObj = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            outputFilename = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-org") == 0) {
            org = argv[i + 1];
            ++i;
        } else if (strcmp(argv[i], "-no-exit") == 0) {
            noexit = true;
        } else if (strcmp(argv[i], "-bits-32") == 0) {
            bits = 32;
        } else if (strcmp(argv[i], "-bits-64") == 0) {
            bits = 64;
        } else if (strcmp(argv[i], "-end-halt") == 0) {
            endhalt = true;
        } else if (argv[i][0] == '-') {
            printf("Invalid argument!\n");
            exit(1);
        } else {
            filename = argv[i];
            break;
        }
    }

    if (access(filename, F_OK) != 0) {
        printf("KessLang: Cannot open \"%s\"! File not found.\n", argv[1]);
        exit(0);
    } 

    FILE* fp = fopen(filename, "r");

    fseek(fp, 0, SEEK_END);
    size_t fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)calloc(fsize + 1, sizeof(char));
    fread(buffer, sizeof(char), fsize, fp);
    fclose(fp);

    tokenlist_t tokenlist = {
        .size = 0,
        .tokens = (token_t*)malloc(sizeof(token_t))
    };

    lexer_t lexer = {
        .idx = 0,
        .tokenlist = tokenlist,
        .lineNum = 1,
        .error = false,
    };

    tokenize(&lexer, buffer);

    if (lexer.error) { 
        destroy_tokenlist(&lexer.tokenlist);
        free(buffer);
        exit(1);
    }

    parser_t parser = {.idx = 0, .tokenlist = lexer.tokenlist};

    parse(&parser);

    if (parser.error) { 
        ast_destroy(&parser.ast);
        destroy_tokenlist(&lexer.tokenlist);
        free(buffer);
        exit(1);
    }
    
    kl_cgen_start(parser.ast);

    ast_destroy(&parser.ast);
    destroy_tokenlist(&lexer.tokenlist);
    free(buffer);

    if (!(codegenError)) {
        if (outputFilename != NULL) {
            char* command;
            if (outputObj) { 
                command = (char*)calloc(snprintf(NULL, 0, "nasm -felf32 /tmp/__KL_SOURCE.S -o $PWD/%s", outputFilename) + 2, sizeof(char));
                sprintf(command, "nasm -felf32 /tmp/__KL_SOURCE.S -o $PWD/%s", outputFilename);
            } else if (!(pureAsm)) {
                command = (char*)calloc(snprintf(NULL, 0, "nasm -felf32 /tmp/__KL_SOURCE.S -o /tmp/__KL_OBJ.o; ld /tmp/__KL_OBJ.o -melf_i386 -o $PWD/%s", outputFilename) + 2, sizeof(char));
                sprintf(command, "nasm -felf32 /tmp/__KL_SOURCE.S -o /tmp/__KL_OBJ.o; ld /tmp/__KL_OBJ.o -melf_i386 -o $PWD/%s", outputFilename);
            } else if (pureAsm) {
                command = (char*)calloc(snprintf(NULL, 0, "mv /tmp/__KL_SOURCE.S $PWD/%s", outputFilename) + 2, sizeof(char));
                sprintf(command, "mv /tmp/__KL_SOURCE.S $PWD/%s", outputFilename);
            }

            system(command);
            free(command);
        } else {
            if (!(pureAsm) && !(outputObj)) {
                system("nasm -felf32 /tmp/__KL_SOURCE.S -o /tmp/__KL_OBJ.o; ld /tmp/__KL_OBJ.o -melf_i386 -o $PWD/a.out");
                remove("/tmp/__KL_OBJ.o"); 
            }

            if (pureAsm) {
                system("mv /tmp/__KL_SOURCE.S $PWD/kl.out.S");
            } else if (outputObj) {
                system("mv /tmp/__KL_OBJ $PWD/kl.out.o");
            }
        }
    }

    if (!(pureAsm)) {
        remove("/tmp/__KL_SOURCE.S");
    }
}
