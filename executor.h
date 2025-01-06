//
// Created by kucia on 12/25/24.
//

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "tokens.h"
#include "print_utils.h"
#include <string.h>
#include <wait.h>
#include <errno.h>

void execute_sequence(struct token**);
void free_sequence(struct token**);

#endif //EXECUTOR_H
