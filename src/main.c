#include <stdarg.h>
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

void eprintf(const char* format, ...) {
  va_list ap;

  va_start(ap, format);
  fprintf(stderr, format, ap);
  va_end(ap);
}

void eputs(const char* msg) {
  fputs(msg, stderr);
  fputc('\n', stderr);
}

bool check_valid_s3m(FILE *S3Mfile) {
  char scrm[4] = {0};

  fseek(S3Mfile, 44, SEEK_SET);

  fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4)) {
    eprintf("This is not an S3M file!");
    return false;
  }

  rewind(S3Mfile);
  return true;
}

int main(int argc, char *argv[]) {
  int return_value = EXIT_SUCCESS;
  FILE *STMfile;
  FILE *S3Mfile;

  if (argc != 3) {
    printf("usage: %s <input.s3m> <output.stm>\n", argv[0]);
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
  if(!check_valid_s3m(S3Mfile)) goto closefiledescriptors;

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
