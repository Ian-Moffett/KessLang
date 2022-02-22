#include "include/util/importparser.h"


char* get_import(char* buffer) {
    static unsigned long idx = 0;
    static bool start = true;
    static bool done = false;

    char* name = (char*)calloc(1, sizeof(char));
    unsigned int nameidx = 0; 

    if (start) {
        // Check if stdinc.
        while (buffer[idx] != '(' && buffer[idx] != '\n' && buffer[idx] != EOF) {
            if (buffer[idx] == ' ') {
                ++idx;
                continue;
            }

            name[nameidx] = buffer[idx];
            ++nameidx;
            ++idx;
            name = (char*)realloc(name, sizeof(char) * (nameidx + 2));
        }

        if (buffer[idx] != '(') {
            return NULL;
        }

        if (strcmp(name, STDINC) != 0) {
            return NULL;
        }

    }

    ++idx;
    name = (char*)realloc(name, sizeof(char));
    nameidx = 0;

    while (idx < strlen(buffer)) {
        if (buffer[idx] == '"' || buffer[idx] == ' ' || buffer[idx] == '\n') {
            printf("Daddy~\n");
            ++idx;
            continue;
        } else if (buffer[idx] == ',') {
            printf("WHY?\n");
            ++idx;
            break;
        }

        if (buffer[idx] == ')') {
        }

        name[nameidx] = buffer[idx];
        ++idx;
        ++nameidx;
        name = (char*)realloc(name, sizeof(char) * (nameidx + 2));
    }

    return name;
}
