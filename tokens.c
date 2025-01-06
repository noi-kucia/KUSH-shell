//
// Created by kucia on 12/1/24.
//

#include "tokens.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "executor.h"

const char *token_type_names[] = {"error", "unknown token", "unfinished sequence", "empty", "command term", "semicolon", "pipe",
"input redirect", "output redirect", "output redirect append", "end"};
const char *command_allowed_symbols = "./~_()-#%^[]+:";
const char *escape_chars = "\\ nrvtbf'\"";
const char *white_characters = " \t\r\n\v\f";
const char *term_terminate_symbols = "|;<>";

struct token next_token_safe(const tchar_t *command) {
    /* Part of the lexer functionality serves for receiving one token.
     * The only argument is a pointer to place to start scanning from.
     * Returns a token struct object, and if for some reason it's impossible,
     * the type will be set to error (empty string or allocation error).
     * If some unknown sequence or character is found, returns a token with token_unknown type and
     * prints an appropriate error message.
     *
     * Note that token.src could not match the argument pointer, cuz white characters can be omitted.
     * Also note that token.src refers to command, so it shouldn't be overridden before token is freed.
     */

    struct token token = {token_error, command, 1};
    if (command==NULL) return token;
    const tchar_t *src = command;
    bool token_found = false;


    while (*src) {  // looking for token start
        token.src = src;
        tchar_t sym = *src++;

        // checking whether symbol is a white character or not
        if (strchr(white_characters, sym)) {
            token.type = token_empty;
            continue;
        }

        // extracting the token depending on the first character
        // interpreting quotes content as command term
        if (sym == '"' || sym == '\'') {
            token.type = token_commandterm;
            while (*(src++)!=sym && *src) token.length++;
            if (!*src && *(src-1)!=sym) { // if no closing quote found
                token.type = token_unfinished;
                token.length = src - token.src;
                error_emph_prefix("Unclosed quote has been found - ", token.src, 0, token.length);
            }
            else token.length++; // adding closing quote to length
        }

        // command terms itself
        else if (isalnum(sym) || strchr(command_allowed_symbols, sym)!=NULL) {
            token.type = token_commandterm;
            token.length = 1;

            while (true) {
                // normal characters
                if (!*src) break;
                if (isalnum(*src) ||  strchr(command_allowed_symbols, *src)!=NULL) {
                    token.length++;
                    src++;
                }
                // escape characters
                else if (*src=='\\') {
                    if (*(src+1)) {
                        if (strchr(escape_chars, *(src+1))!=NULL) {
                            token.length += 2;
                            src += 2;
                        }
                        else {
                            // when a backslash is followed by unknown character
                            error_emph_prefix("Unknown escape character - ", token.src, token.length, token.length+2);
                            token.type = token_error;
                            return token;
                        }
                    }
                    else {
                        // when a backslash is the last character
                        error_emph_prefix("Unterminated escape sequence - ", token.src, token.length, token.length+1);
                        token.type = token_error;
                        return token;

                    }
                }
                else {
                    if (strchr(white_characters, *src)!=NULL || strchr(term_terminate_symbols, *src)!=NULL) break;
                    else {
                        error_emph_prefix("Forbiden character has been found - ", token.src, token.length, token.length+1);
                        token.type = token_error;
                        return token;
                    }
                }
            }
        }

        // operators or unknown
        else {
            switch (sym) {
                case '|':
                    token.type = token_pipe;
                    break;
                case ';':
                    token.type = token_semicolon;
                    break;
                case '<':
                    if (*(src) == '<') {
                        token.type = token_unknown;
                        token.length = 2;
                        error_emph_prefix("Token '<<' is forbidden - ", token.src, 0, 2);
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
                    token.type = token_unknown;
                    error_emph_prefix("Unable to detect token type - ", token.src, 0, 3);
            }
        }
        token_found = true;
        break; // exiting the loop


    }
    if (!token_found) token.type = token_end;

    return token;
}

struct token **get_tokens_safe(const tchar_t *command) {
    /* Reads all the tokens from command and returns a pointer to a null-terminated array of token pointers.
     * In case of error will return NULL.*
     * When unknown or error token is found, NULL is returned, and the corresponding message is printed.
     * All token exceptions should be caught in next_token_safe, not here.
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
    enum token_types previous_type;
    do {
        // getting a token
        token = malloc(sizeof(struct token));
        if (!token) return nullptr;
        *token = next_token_safe(command);  // moving the token into a heap

        // checking is there enough space in the array
        if (tokenc >= token_buffsize) {
            token_buffsize *= 2;
            typeof(tokens) tokens_rltd = realloc(tokens, token_buffsize*sizeof(void*));
            if (tokens_rltd == nullptr) { // reallocation fault
                free(tokens);
                return nullptr;
            }
            tokens = tokens_rltd;
        }

        previous_type = token->type;
        command = token->src + token->length;
        tokens[tokenc] = (previous_type != token_end &&
                          previous_type != token_empty &&
                          previous_type != token_error &&
                          previous_type != token_unknown &&
                          previous_type != token_unfinished)? token : nullptr;

    } while (tokens[tokenc++]);

    // when a sequence isn't correctly finished
    if (previous_type!=token_end && previous_type!=token_empty) {
        error_message("^ unable to parse the "
                      "command, see errors above ^"); // printing a message
        // clearing the memory
        free(token);
        free_sequence(tokens);

        return nullptr;
    }

    free(token);
    return tokens;
}

struct token **get_pipe_segment(struct token **prev_segment) {
    /* Takes a pointer to the previous segment and searches for the next pipe token.
     * Returns a pointer to the token next to the pipe or a nullptr if the pipe wasn't found.
     */

    if (*prev_segment==nullptr) return nullptr;
    struct token **next_segment=prev_segment+1;
    while (*next_segment&&(*(next_segment-1))->type!=token_pipe) next_segment++;
    return *next_segment? next_segment : nullptr;
}

struct token **get_next_command(struct token **prev_command) {
    /* Takes a pointer to the previous command and searches for the next semicolon token.
     * Reruns a pointer to the token next to semicolon or a nullptr if it wasn't found.
     */

    if (*prev_command==nullptr) return nullptr;
    struct token **next_command=prev_command+1;
    while (*next_command&&
           (*next_command)->type!=token_pipe&&
           (*(next_command-1))->type!=token_semicolon) {
        next_command++;
    }
    return *next_command&&(*(next_command-1))->type==token_semicolon? next_command : nullptr;
}

struct token **get_arguments(struct token **command) {
    /* Takes a pointer to a sequence of tokens and returns a pointer to a newly allocated
     * null-terminated array of pointers to tokens that should be passed to exec* function including command name.
     * Will terminate when semicolon, pipe or redirect token is found.
     * If no arguments are found, an array with nullptr will be returned.
     */

    size_t argc=0, arrsize=2;
    struct token **arguments = malloc(arrsize*sizeof(struct token *));
    if (!arguments) return nullptr;
    while(*command && (*command)->type==token_commandterm) {
        if (argc+2>arrsize) {
            arrsize <<= 1;
            struct token **arg_rlctd = realloc(arguments, arrsize*sizeof(struct token *));
            if (arg_rlctd==nullptr) {
                perror("realloc failed");
                exit(15);
            }
            arguments = arg_rlctd;
        }
        arguments[argc++] = *command;
        command++;
    }
    arguments[argc] = nullptr;
    return arguments;
}

char **get_names_after_token(struct token **command, enum token_types type) {
    /* Takes a null-terminated sequence of pointers to tokens and a type number as arguments.
     * Return an array of C-strings representing content of command term tokens
     * following all tokens of a specified type.
     * For instance:
     *   command is [say "hello <3" bla-bla < a.txt b > output < file]
     *   type is token_inredir (< symbol)
     *   than the function will return ["a.txt", "b", "file"]
     *
     * memory should be than freed.
     */

    size_t arrsize = 2, namec = 0;
    char **names = malloc(arrsize*sizeof(char *));

    // going through all tokens till the end
    while (*command) {

        // looking for token with a specified type
        if ((*command)->type==type) {
            // all extracting all names of command terms after this token
            while (*(++command) && (*command)->type==token_commandterm) {
                char *name = malloc(((*command)->length+1 )* sizeof(char));  // allocating memory for a name
                if (name==nullptr) {
                    perror("allocation error");
                    exit(14);
                }

                // reallocating the array if needed
                if (namec+2>arrsize) {
                    arrsize <<= 1;
                    char **names_rlct = realloc(names, arrsize * sizeof(char *));
                    if (names_rlct) {
                        names = names_rlct;
                    } else {
                        free(names);
                        perror("allocation error");
                        exit(13);
                    }
                }
                strncpy(name, (*command)->src, (*command)->length); // extracting the name
                name[(*command)->length] = '\0';
                names[namec++] = name;  // adding a name to the array
            }
        } else command++;
    }

    names[namec] = nullptr;
    return names;
}
