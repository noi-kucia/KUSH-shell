//
// Created by kucia on 12/25/24.
// Provides methods to manage and execute sequences of tokens.
//

#include "executor.h"
#include <stdlib.h>
#include <unistd.h>

extern const char *token_type_names[];

int builtin_cd(char **arguments) {
    int res;
    if (arguments[1]==nullptr) {
        res = chdir(getenv("HOME"));
    }
    else if (arguments[2]!=nullptr) {
        res = 1400;
        perror("cd command takes only 1 argument!");
    }
    else {
        char path[2048];
        if (*arguments[1]=='~') {
            if (*(arguments[1]+1)=='/' || *(arguments[1]+1)=='\0') {
                snprintf(path, sizeof path, "%s%s", getenv("HOME"), arguments[1]+1);
            }
            else {
                perror("unsupported sytax");
                return 9191;
            }
        }
        else {
            snprintf(path, sizeof path, "%s", arguments[1]);
        }
        res = chdir(path);
    }
    return res;
}

void execute_sequence(struct token **sequence) {
    /* Takes a pointer to a sequence of token pointers as argument
     * and executes is as command.
     * If a sequence is wrong, the corresponding message will be printed on diagnostic output.
     */

    int pfd[2];  // pipe
    bool input_pipe_set = false;

    // going through token sequences dividing them by pipes
    for (struct token **segment=sequence;segment!=nullptr;segment=get_pipe_segment(segment)) {
        pipe(pfd); // opening pipe for all output inside this iteration

        // going through individual command within segments between pipes.
        // the command can start with any token and ends with end, semicolon or pipe token (so can be empty)
        for (struct token **command=segment;command!=nullptr;command=get_next_command(command)) {

            /*// debug
            // cprintnl("new command start, arguments are:", Colors.RED);
            // struct token **argv = get_arguments(command);
            // for (typeof(argv)tc=argv;;tc++) {
            //     if (*tc==nullptr) {
            //         // when all arguments were written, freeing the memory and printing redirections
            //         free(argv);
            //         printf("input redirect sources:\n");
            //         auto inredirs = get_names_after_token(command, token_inredir);
            //         for (typeof(inredirs)tc=inredirs;*tc;tc++) {
            //             cprintnl(*tc, Colors.GREEN);
            //         }
            //         printf("output redirect destinations:\n");
            //         auto outredirs = get_names_after_token(command, token_outredir);
            //         for (typeof(outredirs)tc=outredirs;*tc;tc++) {
            //             cprintnl(*tc, Colors.PURPLE);
            //         }
            //         printf("output redirect with append destinations:\n");
            //         auto outredirsap = get_names_after_token(command, token_outredirap);
            //         for (typeof(outredirsap)tc=outredirsap;*tc;tc++) {
            //             cprintnl(*tc, Colors.ORANGE);
            //         }
            //         break;
            //     }
            //     // printing arguments
            //     char token_cont[120];
            //     const struct token token = **tc;
            //     strncpy(token_cont, token.src, token.length);
            //     token_cont[token.length] = '\0';
            //     printf("\t");
            //     cprintnl(token_cont, Colors.YELLOW);
            //     // printf(" - type: %s of length %d\n", token_type_names[token.type], token.length);
            // }
            // cprintnl("command end", Colors.RED);*/

            // getting command arguments
            // TODO: All argument names must be processed to replace ~ by $HOME, delete quotes and replace escape characters
            // TODO: I think it should be done in a new process_argument_names function
            char **arguments;
            if((arguments=get_argument_names(command)) == nullptr) {
                perror("unable to get arguments");
                exit(22);
            }

            // forking to execute a command
            const pid_t pid = fork();
            if (pid < 0) {
                perror("fork failure");
                exit(21);
            }
            if (pid == 0) {
                // child process
                const char *command_name = arguments[0];

                // ignoring empty commands
                if (command_name == nullptr) continue;

                // built-in functions
                if (strcmp(command_name,"cd")==0) {
                    builtin_cd(arguments);
                    continue;
                }
                else if (strcmp(command_name,"help")==0) {
                    // TODO: call built-in help command
                    continue;
                }

                // calling external program otherwise
                dup2(pfd[1], STDOUT_FILENO);
                close(pfd[0]);
                close(pfd[1]);
                execvp(command_name, arguments);
                char msg[0xFF];
                sprintf(msg, "exec failure of  [%s]\nerrno - %d", command_name, errno);
                error_message(msg);

            }
            waitpid(pid, nullptr, 0);
            input_pipe_set = false;  // even if only one command was executed, the next one won't use it again

            // freeing memory
            for (char **arg=arguments;*arg!=nullptr;arg++) free(*arg);
            free(arguments);

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
