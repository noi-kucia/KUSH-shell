//
// Created by kucia on 12/25/24.
// Provides methods to manage and execute sequences of tokens.
//

#include "executor.h"
#include <stdlib.h>
#include <unistd.h>

extern const char *token_type_names[];

void execute_sequence(struct token **sequence) {
    /* Takes a pointer to a sequence of token pointers as argument
     * and executes is as command.
     * If a sequence is wrong, the corresponding message will be printed on diagnostic output.
     */


    // pid_t pid = fork();
    // int ipd[2];
    // if (pid == -1) {
    //     error_message("fork failed");
    //     return;
    // }
    // if (pid == 0) {
    //     // dup2(ipd[1], STDOUT_FILENO);
    //     printf("the child process has just started");
    //     char data[100];
    //     scanf("%s", data);
    //     printf("%s\n", data);
    //     // close(ipd[0]);
    //     // close(opd[1]);
    // }
    // waitpid(pid, NULL, 0);

    int ipfd[2], opfd[2];  // pipes
    bool input_pipe_set = false;

    // going through token sequences dividing them by pipes
    for (struct token **segment=sequence;segment!=nullptr;segment=get_pipe_segment(segment)) {

        // going through individual command within segments between pipes.
        // the command can start with any token and ends with end, semicolon or pipe token (so can be empty)
        for (struct token **command=segment;command!=nullptr;command=get_next_command(command)) {

            // debug
            // cprintnl("new command start", Colors.RED);
            // for (typeof(command)tc=command;*tc;tc++) {
            //     if ((**tc).type==token_semicolon||(**tc).type==token_pipe) {
            //         cprintnl("command end", Colors.RED);
            //         break;
            //     }
            //     char token_cont[120];
            //     const struct token token = **tc;
            //     strncpy(token_cont, token.src, token.length);
            //     token_cont[token.length] = '\0';
            //     printf("\t");
            //     cprint(token_cont, Colors.YELLOW);
            //     printf(" - type: %s of length %d\n", token_type_names[token.type], token.length);
            // }

        }

        // cprintnl("PIPE or an end", Colors.GREEN);  // debug
        input_pipe_set = true;  // in the next ev. iteration input will be redirected from the pipe instead of STDIN
    }

}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
