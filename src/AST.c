#include "include/AST.h"

void ast_push_node(ast_t* ast, ast_node_t node) {
    ast->nodes[ast->size] = node;
    ++ast->size;
    ast->nodes = (ast_node_t*)realloc(ast->nodes, sizeof(ast_node_t) * (ast->size + 2));
}


void node_push_child(ast_node_t* node, node_child_t child) {
    node->children[node->nChild] = child;
    ++node->nChild;
    node->children = (node_child_t*)realloc(node->children, sizeof(node_child_t) * (node->nChild + 2));
}

void ast_destroy(ast_t* ast) {
    for (int i = 0; i < ast->size; ++i) {
        for (int j = 0; j < ast->nodes[i].nChild; ++j) {
            if (ast->nodes[i].children[j].alloc) {
                free(ast->nodes[i].children[j].value);
            }
        }

        if (ast->nodes[i].alloc) {
            free(ast->nodes[i].value);
        }

        free(ast->nodes[i].children);
    }

    free(ast->nodes);
    ast->nodes = NULL;
}


ast_node_t createNode(char* key, char* value, bool alloc, unsigned long lineNum) {
    ast_node_t newNode = {
        .key = key,
        .value = value,
        .alloc = alloc,
        .nChild = 0,
        .children = (node_child_t*)malloc(sizeof(node_child_t)),
        .lineNumber = lineNum
    };

    return newNode;
}


node_child_t createChild(char* key, char* value, bool alloc) {
    node_child_t newChild = {
        .key = key,
        .value = value,
        .alloc = alloc,
    };

    return newChild;
}
