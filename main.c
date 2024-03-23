#include <limits.h>

#if CHAR_BIT != 8
#error "a byte has to be 8-bits!"
#endif

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    printf("Hello, World!\n");
    return EXIT_SUCCESS;
}