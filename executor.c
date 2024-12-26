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
}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
