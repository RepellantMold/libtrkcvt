#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

int main(int argc, char *argv[]) {
  int return_value = EXIT_SUCCESS;
  FILE *STMfile;
  FILE *S3Mfile;
  /* printf("Hello, World!\n"); */

  if (argc != 3) {
    printf("usage: %s <input.s3m> <output.stm>", argv[0]);
    return_value = EXIT_FAILURE;
    goto returndasvalue;
  }

  S3Mfile = fopen(argv[1], "rb");
  STMfile = fopen(argv[2], "wb");

  if (!S3Mfile || !STMfile) {
      return_value = EXIT_FAILURE;
      perror("Failed to open file");
      goto closefiledescriptors;
  }

  /* more code soon */

  closefiledescriptors:
  fclose(S3Mfile);
  fclose(STMfile);

  returndasvalue:
  return return_value;
}