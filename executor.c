//
// Created by kucia on 12/25/24.
// Provides methods to manage and execute sequences of tokens.
//

#include "executor.h"
#include <stdlib.h>
#include <unistd.h>

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

    // we should take care of pipe on the highest level cuz it's an operator with a lowest precedence
    // pipe draft

    // pipe creation
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        error_message("Pipe creation fault");
        return;
    }

    // left side
    pid_t pid1 = fork();
    if (pid1 < 0) {
        error_message("Fork fault");
        return;
    }

    if (pid1 == 0) {
        // child process
        dup2(pipefd[1], STDOUT_FILENO); // redirecting output to the pipe entrance
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("ls", "ls", "-l", NULL);  // example
    }

    waitpid(pid1, NULL, 0);

    // right side
    pid_t pid2 = fork();
    if (pid2 < 0) {
        error_message("Fork fault");
        return;
    }

    if (pid2 == 0) {
        // child process
        dup2(pipefd[0], STDIN_FILENO); // redirecting pipe output to the process input
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("echo", "echo", NULL);  // example
    }

    waitpid(pid2, NULL, 0); // waitpid of pid1 can also be here if we wanna execute 2 processes in parallel
    // if we delete waitpids, those processes will be executed in the background (could be useful later)
}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
