//
// Created by kucia on 12/1/24.
//

#include <stdint.h>
#include <stdio.h>
#include "tokens.h"

#include <ctype.h>
#include <stdlib.h>

extern void error_message(const char* text);

const char *white_characters = " \t\r\n\v\f";

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

    while (src) {  // looking for token start
        tchar_t sym = *src++;

        // checking whether symbol is a white character or not
        bool sh_skip = false;
        for (char *wc=white_characters; *wc; wc++) {
            if (*wc == sym) {
                sh_skip = true;
                break;
            }
        }
        if (sh_skip) continue;

        // extracting the token
        token.src = src-1;
        if (sym == '"' || sym == '\'') {  // interpreting quotes content as command part
            token.type = token_commandpart;
            token.length = 0;
            token.src++; // because the first symbol is a quote
            while (*(src++)!=sym) token.length++;
        }
        else if (isalpha(sym)) {  //  interpreting all alphanumeric words (starting with a letter) as command parts
            token.type = token_commandpart;
            token.length = 1;
            while (isalnum(*(src++))) token.length++;
        }
        else {
            token.type = token_unkown;
            return token;
        }
        break; // exiting the loop


    }

    return token;
};