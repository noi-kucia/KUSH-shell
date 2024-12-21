//
// Created by kucia on 12/1/24.
//

#include <stdint.h>
#include <stdio.h>
#include "tokens.h"

#include <ctype.h>
#include <stdlib.h>

char* token_type_names[] = {"error", "unknown token", "unfinished sequence", "empty", "command part", "semicolon", "pipe",
"input redirect", "input redirect append", "output redirect", "output redirect append", "end"};

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
    if (command==NULL) return token;
    tchar_t *src = command;
    bool token_found = false;


    while (*src) {  // looking for token start
        tchar_t sym = *src++;

        // checking whether symbol is a white character or not
        bool sh_skip = false;
        for (char *wc=white_characters; *wc; wc++) {
            if (*wc == sym) {
                token.type = token_empty;
                sh_skip = token_found = true;
                break;
            }
        }
        if (sh_skip) continue;

        // extracting the token
        token.src = src-1;
        if (sym == '"' || sym == '\'') {  // interpreting quotes content as command part
            token.type = token_commandpart;
            while (*(src++)!=sym && *src) token.length++;
            if (!*src && *(src-1)!=sym) { // if no closing quote found
                token.type = token_unfinished;
                token.length = src - token.src;
            }
            else token.length++; // adding closing quote to length
        }
        else if (isalpha(sym)) {  //  all command parts without quotes (should start with letter)
            // TODO: process backslash characters like \" \\ \[space] and so on
            // TODO: contend could be non-alphanumeric like dots and underscores - a_b.txt
            token.type = token_commandpart;
            token.length = 1;
            while (isalnum(*(src++))) token.length++;
        }
        else {
            switch (sym) {
                case '|':
                    token.type = token_pipe;
                    break;
                case ';':
                    token.type = token_semicolon;
                    break;
                case '<':
                    if (*(src) == '<' && *(src+1)!='<') {
                        token.type = token_inredirap;
                        token.length = 2;
                    }
                    else token.type = token_inredir;
                    break;
                case '>':
                    if (*(src) == '>' && *(src+1)!='>') {
                        token.type = token_outredirap;
                        token.length = 2;
                    }
                    else token.type = token_outredir;
                    break;
                default:
                    token.type = token_unkown;
            }
        }
        token_found = true;
        break; // exiting the loop


    }
    if (!token_found) token.type = token_end;
    else if (token.type == token_empty) {
        token.src = command;
        token.length = src - command;
    }

    return token;
};