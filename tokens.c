//
// Created by kucia on 12/1/24.
//

#include <stdint.h>
#include <stdio.h>

extern void error_message(const char* text);

struct token {

};

void* tokenize(const char* command) {
    /* Tokenize the command and returns a pointer to the array of tokens, that ends with special end-token
     * returns nullptr when unable to parse or allocate enough memory */

    size_t array_size = 32;
    struct token* tokens[array_size];

    return nullptr;
};