cmake_minimum_required(VERSION 3.29)
project(KUSH_shell C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -lm -lncurses")

add_executable(KUSH_shell
    shell.c
        tokens.c
        tokens.h
        executor.c
        executor.h
        print_utils.h
        print_utils.c
        shell.h
        signalsHandling.c
        signalsHandling.h
)
add_executable(test test.c)
