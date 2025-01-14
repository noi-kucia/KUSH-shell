//
// Created by kucia on 12/25/24.
// Provides methods to manage and execute sequences of tokens.
//

#include "executor.h"
#include <stdlib.h>
#include <unistd.h>
#include "shell.h"

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
                perror("unsupported syntax");
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
    printf("KUSH v"); cprintnl(VERSION, Colors.YELLOW);
    cprintnl("\nsupported syntax elements:", Colors.ORANGE);
    printf("* pipes (|) - allows to redirect an stdout from all previous commands before to stdin of the next ones\n");
    printf("* redirection (< or > or >>) operator same as in bash/zsh opens a specified file and replaces std(in/out) with it.\n");
    printf("Has a higher precedence than pipe operator, so if > or >> operator was used, pipe won't get this stdout,\n");
    printf("same situation with input redirections. There could be several operators simultaneously or several file names after every of them.\n");
    printf("In this case, the only thing matters is an order they are specified.\n");
    printf("    Syntax: command arg1 arg2 < b.txt a.txt > out.txt\n");
    printf("    Here stdin will be replaced with a content of b.txt+a.txt (b.txt will be read in first) and stdout will be written into out.txt.\n");
    printf("* semicolons (;) allows to execute several commands sequentially.\n");
    printf("    Syntax: command1;command2;command3\n");
    printf("The stdout of those commands is joined, so if it is redirected by a pipe operator,\n");
    printf("the next command (with not redirected input) will receive on stdin stdout1+stdout2+stdout3.\n");
    printf("\n");
    cprintnl("useful features:", Colors.GREEN);
    printf("* In paths tilda (~) will be replaces with HOME environmental variable path if stands as a first symbol and before a slash (/)\n");
    printf("  for instance builtin_exit:  ~/Desktop/homework.png -> /home/user/homework.png\n");
    printf("* The tokens (arguments and commands) are split by white characters or operators like pipe.\n"
           "  But if you wanna use one of them or just put some huge text, you can use quotes (both ' and \").\n"
           "  The content inside it will be interpreted as a singletoken.");
    printf("* To avoid using quotes, you may use escape characters starting with a backslash.\n"
           "    available characters are: \\\\\\ \\n\\r\\v\\t\\b\\f\\'\\\" \n"
           "    for instance: cd ~/home\\ work/asian/calculus.png\n"
           "* vertical arrows can be used to navigate in previously used commands.\n  By default history size has a limit "
           "of 50 elements.");
    printf("\n");
    cprintnl("built-ins:", Colors.PURPLE);
    printf("* help - command you are using rn\n");
    printf("* cd [path] - command used to change current working directory to path or to the home path if it wasn't specified.\n"
           "* history - command to print list of previously used commands.");
    printf("\n\n");
}

void builtin_exit() {
    cprintnl("\nGoodbye!", Colors.ORANGE);
    exit(EXIT_SUCCESS);
}

void builtin_history() {
    printf("HISTORY:\n");
    for (int i=0;i<history_size;i++) {
        printf("%ld. ", history_size-i);
        printf("%s\n", history[i]);
    }
    printf("\n");
}

void execute_sequence(struct token **sequence) {
    /* Takes a pointer to a sequence of token pointers as argument
     * and executes is as command.
     * If a sequence is wrong, the corresponding message will be printed on diagnostic output.
     */

    int pfd[2], oldfd, opfd[2];  // pipe
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
            char **arguments;
            if((arguments=get_argument_names(command)) == nullptr) {
                perror("unable to get arguments");
                exit(22);
            }

            // processing them
            for(int i=0;arguments[i]!=nullptr;i++) arguments[i]=process_name(arguments[i]);
            const char *command_name = arguments[0];
            if (command_name == nullptr) continue;  // ignoring empty commands

            // getting input redirections
            char **input_names = get_names_after_token(command, token_inredir);
            const bool should_redirect_input = *input_names!=nullptr;

            // getting output redirections
            char **output_names = get_names_after_token(command, token_outredir);
            char **output_names_ap = get_names_after_token(command, token_outredirap);
            const bool should_redirect_output = *output_names||*output_names_ap;
            if (should_redirect_output) pipe(opfd);  //  creating a pipe to redirect the output


            // built-in functions case
            if (strcmp(command_name,"cd")==0) {
                builtin_cd(arguments);
                continue;
            }
            if (strcmp(command_name,"help")==0) {
                builtin_help();
                continue;
            }
            if (strcmp(command_name, "exit")==0) {
                builtin_exit();
                continue;
            }
            if (strcmp(command_name, "history")==0) {
                builtin_history();
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

                // replacing stdin
                if (should_redirect_input) {

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
                if (should_redirect_output) {
                    // redirecting to special pipe that allows to redirect it in the main process to many files
                    dup2(opfd[1], STDOUT_FILENO);
                    close(opfd[1]);
                    close(opfd[0]);
                }
                else {
                    // redirecting to global pipe
                    if (get_pipe_segment(segment)!=nullptr) {  // if there's a pipe after it
                        dup2(pfd[1],STDOUT_FILENO);
                        close(pfd[0]);
                        close(pfd[1]);
                    }
                }

                // exec call
                execvp(command_name, arguments);
                char msg[0xFF];
                sprintf(msg, "exec failure of [%s]", command_name);
                perror(msg);
                _exit(170);

            }

            // waiting for the process end
            int status;
            waitpid(pid, &status, 0);

            // redirecting output to specified files from opfd
            if (should_redirect_output) {
                close(opfd[1]);
                // opening all files (total mess)
                size_t olen=0, oaplen=0;
                ssize_t bytes_read;
                size_t buff_size = 128;
                char buffer[buff_size];
                while ((bytes_read = read(opfd[0], buffer, buff_size)) > 0){
                    // Write to all files
                    for (char **name=output_names;*name;name++) {
                        const int fptr = open(*name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
                        if (write(fptr, buffer, bytes_read) != bytes_read) {
                            perror("writing to file error (output redirection)");
                        }
                        close(fptr);
                    }
                    for (char **name=output_names_ap;*name;name++) {
                        const int fptr = open(*name, O_WRONLY | O_CREAT | O_APPEND, 0666);
                        if (write(fptr, buffer, bytes_read) != bytes_read) {
                            perror("writing to file error (output redirection)");
                        }
                        close(fptr);
                    }
                }
                close(opfd[0]);

            }

            // if there were no redirections but an input pipe, it was used instead and now must be closed
            if (!should_redirect_input && input_pipe_set) {
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

            // if a command in a sequence fails, it's too dangerous to continue execution
            if (WIFEXITED(status) && WEXITSTATUS(status)==49322) {
                close(pfd[1]);
                close(pfd[0]);
                if (input_pipe_set) close(oldfd);
                return;
            }

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
