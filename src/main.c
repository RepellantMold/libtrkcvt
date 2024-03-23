#include "envcheck.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    /* printf("Hello, World!\n"); */
	
	if (argc != 3)
		printf("usage: %s <input.s3m> <output.stm>", argv[0]);
	
	
	
    return EXIT_SUCCESS;
}