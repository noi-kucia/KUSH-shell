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
    // printf("tokens:\n");
    // for (typeof(sequence)tc=sequence;*tc;tc++) {
    //     char token_cont[120];
    //     const struct token token = **tc;
    //     strncpy(token_cont, token.src, token.length);
    //     token_cont[token.length] = '\0';
    //     cprint(token_cont, Colors.YELLOW);
    //     printf(" - type: %s of length %d\n", token_type_names[token.type], token.length);
    // }

    while (*sequence) {  // going through individual commands

        // extracting the command (always first)
        const struct token command = **(sequence);
        if (command.type!=token_commandterm) {
            char mesg[256];
            snprintf(mesg, 256, "Invalid command %s", command.src);
            error_message(mesg);
            break;
        }
        else {
            char command_name[command.length+1];
            command_name[command.length] = '\0';
            strncpy(command_name, command.src, command.length);
            printf("executing command: ");
            cprintnl(command_name, Colors.CYAN);
            break;
        }
    }
}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
