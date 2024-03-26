#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "header.c"
#include "parapnt.c"
#include "sample.c"
#include "pattern.c"

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
  if(check_valid_s3m(S3Mfile) != 0) goto closefiledescriptors;

  fread(s3m_song_header, sizeof(u8), 96, S3Mfile);
  order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  pattern_count = s3m_song_header[36];
  show_s3m_song_header();

  convert_song_header();
  fwrite(stm_song_header, sizeof(u8), 48, STMfile);


  closefiledescriptors:
  fclose(S3Mfile);
  fclose(STMfile);

  returndasvalue:
  return return_value;
}