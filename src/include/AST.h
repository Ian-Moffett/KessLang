#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>


typedef struct {
    char* key;
    char* value;
    bool alloc;
} node_child_t;


typedef struct {
    char* key;
    char* value;
    node_child_t* children;
    unsigned int nChild;
    bool alloc;
    unsigned long lineNumber;
} ast_node_t;


typedef struct {
    unsigned long size;
    ast_node_t* nodes;
} ast_t;


void ast_push_node(ast_t* ast, ast_node_t node);
void node_push_child(ast_node_t* node, node_child_t child);
void ast_destroy(ast_t* ast);
ast_node_t createNode(char* key, char* value, bool alloc, unsigned long lineNum);
node_child_t createChild(char* key, char* value, bool alloc);

#endif
