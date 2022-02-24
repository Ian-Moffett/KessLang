#include "include/CodeGen.h"

extern bool codegenError;
extern char* org;
extern bool noexit;
extern bool endhalt;
extern unsigned char bits;

static int hashmap_hash(const char* key, const int MAX_SIZE) {
    int sum = 0;

    for (int i = 0; i < strlen(key); ++i) {
        sum += key[i];
    }

    if (MAX_SIZE == 0) {
        return -1;
    }

    return sum % MAX_SIZE;
}


void kl_cgen_start(ast_t ast) {
    ast_node_t curNode;
    section_t curSection = CODE_SEC;

    unsigned long sidx = 0;
    unsigned long curLabel = 0;

    func_t curFunction = FUNC_NONE;

    if (access("/tmp/__KL_SOURCE.S", F_OK) == 0) {
        remove("/tmp/__KL_SOURCE.S");
    }

    FILE* fp = fopen("/tmp/__KL_SOURCE.S", "w");
    
    fprintf(fp, "; This assembly program was\n"
                "; generated automatically by\n"
                "; the KessLang compiler which\n"
                "; is written by Ian Moffett.\n\n");

    if (bits == 32) {
        fprintf(fp, "bits 32\n");
    } else if (bits == 64) {
        fprintf(fp, "bits 64\n");
    }

    if (org) {
        fprintf(fp, "org %s\n\n", org);
    }

    fprintf(fp, "global _start\n\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "_start: jmp _%d\n\n", curLabel);

    unsigned long varCount = 0;
    unsigned long funcCount = 0;

    for (int i = 0; i < ast.size; ++i) {
        if (strcmp(ast.nodes[i].key, "VAR") == 0) {
            ++varCount;
        } else if (strcmp(ast.nodes[i].key, "FUNC") == 0) {
            ++funcCount;
        }
    }

    const unsigned long nVarArrSz = varCount;
    const unsigned long nFuncArrSize = funcCount;

    var_t vars[nVarArrSz];
    function_t functions[nFuncArrSize];

    for (int i = 0; i < nVarArrSz; ++i) {
        vars[i].used = false;
        vars[i].count = 1;
    }

    for (int i = 0; i < nFuncArrSize; ++i) {
        functions[i].used = false;
        functions[i].assembly = false;
    }

    for (int i = 0; i < ast.size && !(codegenError); ++i) {
        curNode = ast.nodes[i];

        if (strcmp(curNode.key, "PRINT") == 0) {
            if (curSection != CODE_SEC) {
                curSection = CODE_SEC;
                fprintf(fp, "section .text\n");
            }

            fprintf(fp, "_%d: jmp _%d\n\n", curLabel, curLabel + 2);
            ++curLabel;

            if (curSection != DATA_SEC) {
                curSection = DATA_SEC;
                fprintf(fp, "section .data\n"); 
            }

            fprintf(fp, "_%d: db \"%s\", 0xA\n\n", curLabel, curNode.value);
            ++curLabel;

            curSection = CODE_SEC;
            fprintf(fp, "section .text\n");
            fprintf(fp, "_%d:\n", curLabel);
            fprintf(fp, "    mov eax, 4\n");
            fprintf(fp, "    mov ebx, 1\n");
            fprintf(fp, "    mov ecx, _%d\n", curLabel - 1);
            fprintf(fp, "    mov edx, %d\n", strlen(curNode.value) + 1);
            fprintf(fp, "    int 0x80\n\n"); 
            ++curLabel;
        } else if (strcmp(curNode.key, "VAR") == 0) {
            /* 
             *  The symbol is contained in curNode.value
             *  and the symbol value is contained in
             *  curNode.children[0].value
             */

            if (codegenError) {
                break;
            }
            
            if (curSection != BSS_SEC) {
                curSection = BSS_SEC;
            }

            ast_node_t lastVarRef;

            for (int j = i; j < ast.size; ++j) {
                if (strcmp(ast.nodes[j].value, curNode.value) == 0) {
                    lastVarRef = ast.nodes[j];
                }
            }

            if (strcmp(curNode.children[0].key, "STR") != 0) {
                fprintf(fp, "section .bss\n");
            } else if (!(vars[hashmap_hash(curNode.value, nVarArrSz)].used)) {
                curSection = DATA_SEC;
                fprintf(fp, "section .data\n");
                fprintf(fp, "%s: db \"%s\"\n\n", curNode.value, curNode.children[0].value);
            } else {
                curSection = DATA_SEC;
                fprintf(fp, "section .data\n");
                fprintf(fp, "%s_%d: db \"%s\"\n\n", curNode.value, vars[hashmap_hash(curNode.value, nVarArrSz)].count, curNode.children[0].value);
            }

            if (strcmp(curNode.children[0].key, "INT") == 0) {
                if (!(vars[hashmap_hash(curNode.value, nVarArrSz)].used)) {
                    fprintf(fp, "%s: resb 4\n\n", curNode.value);
                    curSection = CODE_SEC;
                    fprintf(fp, "section .text\n");
                    fprintf(fp, "_%d:\n", curLabel);
                    ++curLabel;

                    fprintf(fp, "    mov [%s], dword %s   ; Stored in executable to use special runtime operations against var. \n\n", curNode.value, curNode.children[0].value);
                } else {
                    fprintf(fp, "%s_%d: resb 4\n\n", curNode.value, vars[hashmap_hash(curNode.value, nVarArrSz)].count);
                    curSection = CODE_SEC;
                    fprintf(fp, "section .text\n");
                    fprintf(fp, "_%d:\n", curLabel);
                    ++curLabel;

                    fprintf(fp, "    mov [%s_%d], dword %s   ; Stored in executable to use special runtime operations against var. \n\n", curNode.value, vars[hashmap_hash(curNode.value, nVarArrSz)].count, curNode.children[0].value);
                    ++vars[hashmap_hash(curNode.value, nVarArrSz)].count;
                }
                
            }
            
            if (strcmp(curNode.children[0].key, "INT") == 0) {
                vars[hashmap_hash(curNode.value, nVarArrSz)].strVal = curNode.children[0].value;
                vars[hashmap_hash(curNode.value, nVarArrSz)].numericVal = atoi(curNode.children[0].value);
                vars[hashmap_hash(curNode.value, nVarArrSz)].isNumeric = true;
            } else {
                vars[hashmap_hash(curNode.value, nVarArrSz)].strVal = curNode.children[0].value;
                vars[hashmap_hash(curNode.value, nVarArrSz)].isNumeric = false;
            }


            vars[hashmap_hash(curNode.value, nVarArrSz)].used = true;
        } else if (strcmp(curNode.key, "VAR_PRINT") == 0) {
            bool varFound = false;
            bool numeric = false;

            if (nVarArrSz != 0) {
                if (vars[hashmap_hash(curNode.value, nVarArrSz)].used) {
                    if (vars[hashmap_hash(curNode.value, nVarArrSz)].isNumeric) {
                        numeric = true;
                    }

                    varFound = true;
                } 
            }

            if (!(varFound)) {
                kl_log_err("SymbolError: Trying to print non-existing variable!", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }


            const char* value2Print = vars[hashmap_hash(curNode.value, nVarArrSz)].strVal; 

            if (curSection != CODE_SEC) {
                curSection = CODE_SEC;
                fprintf(fp, "section .text\n");
            }

            fprintf(fp, "_%d: jmp _%d\n\n", curLabel, curLabel + 2);
            ++curLabel;

            fprintf(fp, "section .data\n");
            fprintf(fp, "_%d: db \"%s\", 0xA\n\n", curLabel, value2Print);
            ++curLabel;

            fprintf(fp, "section .text\n");

            fprintf(fp, "_%d:\n", curLabel);
            fprintf(fp, "    mov eax, 4\n");
            fprintf(fp, "    mov ebx, 1\n");
            fprintf(fp, "    mov ecx, _%d      ; Value stored in hashmap during compile time. (STRICTLY FOR PRINTING ONLY).\n", curLabel - 1);
            fprintf(fp, "    mov edx, %d\n\n", strlen(value2Print) + 1); 
            fprintf(fp, "    int 0x80\n\n");
            ++curLabel;
        } else if (strcmp(curNode.key, "DEREF") == 0) {
            if (curSection != CODE_SEC) {
                curSection = CODE_SEC;
                fprintf(fp, "section .text\n");
            }

            fprintf(fp, "_%d:\n", curLabel);
            ++curLabel;
            fprintf(fp, "    mov [%s], byte \"%s\"\n\n", curNode.value, curNode.children[0].value);            
        } else if (strcmp(curNode.key, "DEREF_VAR") == 0) {
            if (!(vars[hashmap_hash(curNode.value, nVarArrSz)].used) || nVarArrSz == 0) {
                kl_log_err("SymbolError: No dereferenceable variable by that name.", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }

            if (curSection != CODE_SEC) {
                curSection = CODE_SEC;
                fprintf(fp, "section .text\n");
            }

            if (strcmp(curNode.children[0].key, "STR") == 0 && strlen(curNode.children[0].value) > 1) {                
                kl_log_err("SyntaxError: Expected char.", curNode.children[0].value, curNode.lineNumber);
                codegenError = true;
                break;
            }

            fprintf(fp, "_%d:\n", curLabel, curLabel);
            ++curLabel;
            fprintf(fp, "    mov ebx, [%s]\n", curNode.value);
            fprintf(fp, "    mov [ebx], byte \"%s\"\n\n", curNode.children[0].value);
        } else if (strcmp(curNode.key, "FUNC") == 0) {
            // TODO: Add handling for regular functions when that gets added.
            curFunction = FUNC_ASM;

            if (functions[hashmap_hash(curNode.value, nFuncArrSize)].used) {                
                kl_log_err("SymbolError: Defined the same function twice!", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }
    
            if (curSection != CODE_SEC) {
                fprintf(fp, "section .text\n");
                curSection = CODE_SEC;
            }

            fprintf(fp, "_%d:\n", curLabel);
            fprintf(fp, "    jmp _%d\n\n", curLabel + 1);
            ++curLabel;
            fprintf(fp, "f_%s:\n", curNode.value);

            for (int j = i; j < ast.size; ++j, ++i) {
                curNode = ast.nodes[j];
                if (strcmp(curNode.key, "STR") == 0) {
                    fprintf(fp, "    %s\n", curNode.value);
                } else if (strcmp(curNode.value, "__GLB_SCOPE_START") == 0) {
                    curFunction = FUNC_NONE;
                    break;
                }

                continue;
            }

            fprintf(fp, "    ret\n\n");
            functions[hashmap_hash(curNode.value, nFuncArrSize)].used = true;
        } else if (strcmp(curNode.key, "CALL") == 0) {
            if (!(functions[hashmap_hash(curNode.value, nFuncArrSize)].used)) {                
                kl_log_err("SymbolError: No function by that name.", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }

            if (curSection != CODE_SEC) {
                fprintf(fp, "section .text\n");
                curSection = CODE_SEC;
            }

            fprintf(fp, "_%d:\n", curLabel);
            fprintf(fp, "    call f_%s\n\n", curNode.value);
            ++curLabel;
        }
    } 
    
    // END OF CODE (SYS_EXIT).
    if (curSection != CODE_SEC) {
        curSection = CODE_SEC;
        fprintf(fp, "section .text\n"); 
    }

    fprintf(fp, "_%d:\n", curLabel);
    
    if (!(noexit) && !(endhalt)) {
        fprintf(fp, "    mov eax, 1\n");
        fprintf(fp, "    mov ebx, 0\n");
        fprintf(fp, "    int 0x80\n");
    } else if (!(endhalt)) {
        fprintf(fp, "    ret\n");
    } else {
        fprintf(fp, "    cli\n    hlt\n");
    }

    fclose(fp);
}
