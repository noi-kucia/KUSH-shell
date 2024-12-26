//
// Created by kucia on 12/25/24.
// Provides methods to manage and execute sequences of tokens.
//

#include "executor.h"
#include <stdlib.h>

// ; priority is lower than | or >/<

void execute_sequence(struct token** sequence) {
    /* Takes a pointer to a sequence of token pointers as argument
     * and executes is as command.
     * If a sequence is wrong, the corresponding message will be printed on diagnostic output.
     */
    //debug
    perror("example error");
    printf("tokens:\n");
    for (typeof(sequence)tc=sequence;*tc;tc++) {
        char token_cont[120];
        const struct token token = **tc;
        strncpy(token_cont, token.src, token.length);
        token_cont[token.length] = '\0';
        cprint(token_cont, Colors.YELLOW);
        printf(" - type: %s of length %d\n", token_type_names[token.type], token.length);
    }
}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
