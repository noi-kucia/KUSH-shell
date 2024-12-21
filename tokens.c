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
    token_error,
    token_unkown,
    token_command,
    token_argument,
    token_semicolon,
    token_pipe,
    token_inredir,
    token_outredir
};


struct token{
    enum token_types type;
    const tchar_t *src;
    uint16_t length;
};

struct token next_token(const tchar_t *command) {
    /* Part of the lexer functionality serves for receiving one token.
     * The only argument is a pointer to place to start scanning from.
     * Returns a token struct object, and if for some reason it's impossible,
     * the type will be set to error (empty string or allocation error).
     * If some unknown sequence or character is found, returns a token with token_unknown type.
     *
     * Note that token.src could not match the argument pointer, cuz white characters can be omitted.
     */

    struct token token = {token_error, command, 1};
    tchar_t *src = command;

    return token;
};