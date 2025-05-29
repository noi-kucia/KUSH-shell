//
// Created by kucia on 1/17/25.
//

#include "signalsHandling.h"
#include "executor.h"

void on_sigint(int sig) {
    // Forwards SIGINT to the subprocess
    if (subprocess_pid > 0) {
        if (kill(subprocess_pid, SIGINT) == -1) {
            perror("Failed to redirect SIGINT to subprocess");
            subprocess_pid = 0;
        }
        printf("\nProcess %d was killed\n", subprocess_pid);
    }
}

void on_sigstop(int sig) {
    // Forwards SIGSTOP to the subprocess
    if (subprocess_pid > 0) {
        if (kill(subprocess_pid, SIGTSTP) == -1) {
            perror("Failed to redirect SIGTSTP to subprocess");
        }
        printf("\nProcess %d was stopped\n", subprocess_pid);
    }
}

void start_signal_handling() {
    struct sigaction sa;

    // Handling Ctrl+C (SIGINT)
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Failed to set SIGINT handler");
    }

    // Handling Ctrl+Z (SIGTSTP)
    sa.sa_handler = on_sigstop;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("Failed to set SIGTSTP handler");
    }
}