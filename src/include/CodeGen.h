#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"
#include "Debugger.h"
#include "Var.h"
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

typedef enum {
    CODE_SEC,
    DATA_SEC,
    BSS_SEC
} section_t;


typedef enum {
    FUNC_REG,       // Regular.
    FUNC_ASM,
    FUNC_NONE,
} func_t;


void kl_cgen_start(ast_t ast);


#endif
