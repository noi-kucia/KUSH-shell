//
// Created by kucia on 12/1/24.
//

#include <stdint.h>
#include <stdio.h>
#include "tokens.h"
#include <string.h>

#include <ctype.h>
#include <stdlib.h>

char* token_type_names[] = {"error", "unknown token", "unfinished sequence", "empty", "command term", "semicolon", "pipe",
"input redirect", "input redirect append", "output redirect", "output redirect append", "end"};
const char* command_allowed_symbols = "./~\\_()-";

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
    const tchar_t *src = command;
    bool token_found = false;


    while (*src) {  // looking for token start
        tchar_t sym = *src++;

        // checking whether symbol is a white character or not
        bool sh_skip = false;
        for (const char *wc=white_characters; *wc; wc++) {
            if (*wc == sym) {
                token.type = token_empty;
                sh_skip = token_found = true;
                break;
            }
        }
        if (sh_skip) continue;

        // extracting the token
        token.src = src-1;
        if (sym == '"' || sym == '\'') {  // interpreting quotes content as command term
            token.type = token_commandterm;
            while (*(src++)!=sym && *src) token.length++;
            if (!*src && *(src-1)!=sym) { // if no closing quote found
                token.type = token_unfinished;
                token.length = src - token.src;
            }
            else token.length++; // adding closing quote to length
        }
        else if (isalpha(sym) || strchr(command_allowed_symbols, sym)!=NULL) {  // command terms itself
            token.type = token_commandterm;
            token.length = 1;
            while (*src && (isalnum(*src) || strchr(command_allowed_symbols, *src)!=NULL || *(src-1)=='\\')) {
                token.length++;
                src++;
            }
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

struct token **get_tokens(const tchar_t *command) {
    /* Reads all the tokens from command and returns a pointer to a null-terminated array of token pointers.
     * In case of error will return NULL.*
     * Will also ignore all unknown, error or end tokens (they terminate function loop)
     *
     * Note: remember to free the array and its elements!
     */

    // initializing a buffer for token pointers
    size_t token_buffsize = 64;
    struct token **tokens;
    if ((tokens=malloc(token_buffsize*sizeof(void*))) ==  nullptr) return nullptr;  // returns null when cannot allocate memory

    // extracting tokens into the array
    uint16_t tokenc = 0;  // counter of how much tokens are already in array
    struct token *token;
    do {
        // getting a token
        token = malloc(sizeof(struct token));
        if (!token) return nullptr;
        *token = next_token(command);  // moving the token into a heap

        // checking is there enough space in the array
        if (tokenc >= token_buffsize) {
            token_buffsize *= 2;
            if (realloc(tokens, token_buffsize*sizeof(void*)) == nullptr) { // reallocation fault
                free(tokens);
                return nullptr;
            }
        }

        // checking whether the token is terminating
        const enum token_types pt = token->type;
        command = token->src + token->length;
        tokens[tokenc] = (pt != token_end && pt != token_error && pt != token_unkown)? token : nullptr;

    } while (tokens[tokenc++]);
    free(token);

    return tokens;
}