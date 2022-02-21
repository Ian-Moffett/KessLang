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

    for (int i = 0; i < ast.size; ++i) {
        if (strcmp(ast.nodes[i].key, "VAR") == 0) {
            ++varCount;
        }
    }

    const unsigned long nVarArrSz = varCount;

    const char* numericVars[nVarArrSz];
    const char* vars[nVarArrSz];

    for (int i = 0; i < nVarArrSz; ++i) {
        numericVars[i] = NULL;
        vars[i] = NULL;
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

            if (strcmp(curNode.children[0].key, "INT") == 0) {
                if (numericVars[hashmap_hash(curNode.value, nVarArrSz)] != NULL) {
                    kl_log_err("SymbolError: Double symbol refrence's are not supported yet, coming soon..", curNode.value, curNode.lineNumber);
                    codegenError = true;
                    break;
                }
            }

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
            } else {
                curSection = DATA_SEC;
                fprintf(fp, "section .data\n");
                fprintf(fp, "%s: db \"%s\"\n\n", curNode.value, curNode.children[0].value);
            }

            if (strcmp(curNode.children[0].key, "INT") == 0) {
                fprintf(fp, "%s: resb 4\n\n", curNode.value);
                curSection = CODE_SEC;
                fprintf(fp, "section .text\n");
                fprintf(fp, "_%d:\n", curLabel);
                ++curLabel;

                fprintf(fp, "    mov [%s], dword %s   ; Stored in executable to use special runtime operations against var. \n\n", curNode.value, curNode.children[0].value);
                
            }
            
            if (strcmp(curNode.children[0].key, "INT") == 0) {
                numericVars[hashmap_hash(curNode.value, nVarArrSz)] = curNode.children[0].value;
            } else {
                vars[hashmap_hash(curNode.value, nVarArrSz)] = curNode.children[0].value;
            }
        } else if (strcmp(curNode.key, "VAR_PRINT") == 0) {
            bool varFound = false;
            bool numeric = false;

            if (nVarArrSz != 0) {
                if (vars[hashmap_hash(curNode.value, nVarArrSz)] != NULL) {
                    varFound = true;
                } else if (numericVars[hashmap_hash(curNode.value, nVarArrSz)] != NULL) {
                    varFound = true;
                    numeric = true;
                }
            }

            if (!(varFound)) {
                kl_log_err("SymbolError: Trying to print non-existing variable!", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }


            // TODO: Turn this into a hashmap.

            const char* value2Print;

            if (numeric) {
                value2Print = numericVars[hashmap_hash(curNode.value, nVarArrSz)];
            } else {
                value2Print = vars[hashmap_hash(curNode.value, nVarArrSz)];
            }

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

            /*
            if (!(numericVars[hashmap_hash(curNode.value, nVarArrSz)])) {
                kl_log_err("SymbolError: No dereferenceable variable by that name.", curNode.value, curNode.lineNumber);
                codegenError = true;
                break;
            }
            */


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
