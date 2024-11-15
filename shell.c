#include <stdio.h>
#include <stdint.h>
#include <string.h>

void print_greetings(void){
    unsigned int x, y = 5;
    printf("int is %3u and the second one  is %3u\n", x, y);
    printf("hello!");
}

int main(void){
    printf("\n");
    print_greetings();
    return 0;
}