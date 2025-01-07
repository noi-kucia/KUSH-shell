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

void builtin_help() {
    /* prints a help message */
    printf("\n\n");
    printf("KUSH v"); cprintnl("0.5.1", Colors.YELLOW);
    printf("\nsupported syntax elements:\n");
    cprint("pipes", Colors.CYAN);
    printf(" (|) - allows to redirect an stdout from all previous commands before to stdin of the next ones\n");
    cprint("redirection", Colors.CYAN);
    printf(" (< or > or >>) operator same as in bash/zsh opens a specified file and replaces std(in/out) with it.\n");
    printf("Has a higher precedence than pipe operator, so if > or >> operator was used, pipe won't get this stdout,\n");
    printf("same situation with input redirections. There could be several operators simultaneously or several file names after every of them.\n");
    printf("In this case, the only thing matters is an order they are specified.\n");
    printf("    Syntax: command arg1 arg2 < b.txt a.txt > out.txt\n");
    printf("    Here stdin will be replaced with a content of b.txt+a.txt (b.txt will be read in first) and stdout will be written into out.txt.\n");
    cprint("semicolons", Colors.CYAN);
    printf(" (;) allows to execute several commands sequentially.\n");
    printf("    Syntax: command1;command2;command3\n");
    printf("The stdout of those commands is joined, so if it is redirected by a pipe operator,\n");
    printf("the next command (with not redirected input) will receive on stdin stdout1+stdout2+stdout3.\n");
    printf("\n");
}

void execute_sequence(struct token **sequence) {
    /* Takes a pointer to a sequence of token pointers as argument
     * and executes is as command.
     * If a sequence is wrong, the corresponding message will be printed on diagnostic output.
     */

    int pfd[2], oldfd;  // pipe
    bool input_pipe_set = false;

    // going through token sequences dividing them by pipes
    for (struct token **segment=sequence;segment!=nullptr;segment=get_pipe_segment(segment)) {

        // opening a pipe to redirect all output to
        if (pipe(pfd)) {
            perror("pipe failed");
            exit(25);
        }

        // going through individual command within segments between pipes.
        // the command can start with any token and ends with end, semicolon or pipe token (so can be empty)
        for (struct token **command=segment;command!=nullptr;command=get_next_command(command)) {

            // getting command arguments
            // TODO: All argument names must be processed to replace ~ by $HOME, delete quotes and replace escape characters
            // TODO: I think it should be done within function that creates token of commandterm type
            char **arguments;
            if((arguments=get_argument_names(command)) == nullptr) {
                perror("unable to get arguments");
                exit(22);
            }
            const char *command_name = arguments[0];
            if (command_name == nullptr) continue;  // ignoring empty commands

            // getting input redirections
            char **input_names = get_names_after_token(command, token_inredir);
            const bool should_take_redirection = *input_names? true : false;

            // getting output redirections
            char **output_names = get_names_after_token(command, token_outredir);
            char **output_names_ap = get_names_after_token(command, token_outredirap);
            const bool should_take_output = *output_names||*output_names_ap? true : false;

            // built-in functions case
            if (strcmp(command_name,"cd")==0) {
                builtin_cd(arguments);
                continue;
            }
            if (strcmp(command_name,"help")==0) {
                builtin_help();
                continue;
            }

            // forking to execute a command
            const pid_t pid = fork();
            if (pid < 0) {
                perror("fork failure");
                exit(21);
            }
            if (pid == 0) {
                // child process

                // replacing stdin if needed
                if (should_take_redirection) {

                    // creating a pipe to redirect all inputs through
                    int fd[2];
                    if (pipe(fd)) {
                        perror("pipe failed");
                        exit(31);
                    }

                    // reading files to the pipe
                    for (char **name=input_names;*name;name++) {
                        const int fptr = open(*name, O_RDONLY);
                        if (!fptr) {
                            char msg[256];
                            sprintf(msg, "file doesn't exist - %s", *name);
                            perror(msg);
                            continue;
                        }
                        constexpr ssize_t buffer_size = 256;
                        ssize_t bytes_read = 0;
                        char buffer[buffer_size];
                        while ((bytes_read = read(fptr, buffer,  buffer_size)) > 0) {
                            write(fd[1], buffer, bytes_read);
                        }
                        if (bytes_read==-1) {
                            perror("error when reading one of the input files");
                        }
                        close(fptr);
                    }

                    // replacing input with a pipe
                    dup2(fd[0],STDIN_FILENO);
                    close(fd[0]);
                    close(fd[1]);
                }
                else if (input_pipe_set) {
                    // if there's no redirection, but the input pipe is set, replacing stdin with it
                    dup2(oldfd, STDIN_FILENO);
                    close(oldfd);
                }

                // replacing stdout
                dup2(pfd[1],STDOUT_FILENO);
                close(pfd[0]);
                close(pfd[1]);

                // exec call
                execvp(command_name, arguments);
                char msg[0xFF];
                sprintf(msg, "exec failure of  [%s]", command_name);
                perror(msg);
                return; // if a command in a sequence fails, it's too dangerous to continue execution

            }

            // waiting for the process end
            waitpid(pid, nullptr, 0);

            // if there were no redirections but an input pipe, it was used instead and now must be closed
            if (!should_take_redirection && input_pipe_set) {
                input_pipe_set = false;
                close(oldfd);
            }

            // freeing memory
            for (char **arg=arguments;*arg!=nullptr;arg++) free(*arg);
            for (char **name=input_names;*name;name++) free(*name);
            for (char **name=output_names;*name;name++) free(*name);
            free(arguments);
            free(input_names);
            free(output_names);

        }

        close(pfd[1]);
        if (input_pipe_set) close(oldfd); // if it wasn't used and therefore closed in a process before
        input_pipe_set = true;  // in the next ev. iteration input will be redirected from the pipe instead of STDIN
        oldfd = pfd[0];  // file descriptor to be read as input in the next iteration (or will be flushed to the STDOUT)
    }

    // flushing pipe output to the STDOUT
    ssize_t bytes_read;
    size_t buff_size = 128;
    char buffer[buff_size];
    while ((bytes_read = read(oldfd, buffer, buff_size)) > 0){
        // Write to stdout
        if (write(STDOUT_FILENO, buffer, bytes_read) != bytes_read) {
            perror("flush error");
            exit(47);
        }
    }

}

void free_sequence(struct token** sequence) {
    /* Used to free heap memory allocated in get_tokens() function */
    for (typeof(sequence) sq=sequence;*sq;sq++) free(*sq);
    free(sequence);
}
