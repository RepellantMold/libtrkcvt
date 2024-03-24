#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "header.c"
#include "sample.c"
#include "pattern.c"

int main(int argc, char *argv[]) {
  int return_value = EXIT_SUCCESS;
  FILE *STMfile;
  FILE *S3Mfile;
  char scrm[4] = {0};
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
  fseek(S3Mfile, 44, SEEK_SET);

  fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4) != 0) {
    fprintf(stderr, "Not an S3M file!");
    return_value = EXIT_FAILURE;
    goto closefiledescriptors;
  }

  rewind(S3Mfile);

  fread(s3m_song_header, sizeof(u8), 96, S3Mfile);
  show_s3m_header();


  closefiledescriptors:
  fclose(S3Mfile);
  fclose(STMfile);

  returndasvalue:
  return return_value;
}