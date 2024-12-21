//
// Created by kucia on 12/1/24.
//

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern void error_message(const char* text);

// defining types
typedef char tchar_t;  // type of token character
enum token_types{
    token_unkown,
    token_command,
    token_argument,
    token_semicolon,
    token_pipe,
    token_inredir,
    token_outredir
};


struct token {
    enum token_types type;
    const tchar_t* src;
    uint16_t length;
};

struct token** tokenize(const char* command) {
    /* Tokenize the command and returns a pointer to the array of tokens, that ends with special end-token
     * returns nullptr when unable to parse or allocate enough memory */

    size_t array_size = 32;
    struct token* tokens = malloc(sizeof(struct token) * array_size);

    return tokens;
};