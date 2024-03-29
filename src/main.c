/*
 * s3m2stm
 * Made by RepellantMold/cs127 under ISC license
 *
 * Written in C89, with 16-bit DOS compilers in mind
 */


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
  vfprintf(stderr, format, ap);
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

int convert_s3m_to_stm(FILE *S3Mfile, FILE *STMfile) {
  usize i = 0;

  if(!S3Mfile ||!STMfile) return 2;
  if(!check_valid_s3m(S3Mfile)) return 1;

  fread(s3m_song_header, sizeof(u8), 96, S3Mfile);
  order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  pattern_count = s3m_song_header[36];
  show_s3m_song_header();
  check_s3m_channels();

  convert_song_header();
  fwrite(stm_song_header, sizeof(u8), sizeof(stm_song_header), STMfile);

  grab_s3m_parapointers(S3Mfile);

  for(; i < 31; i++) {
    if(i < sample_count) {
      grab_sample_data(S3Mfile, s3m_inst_pointers[i]);
      show_s3m_inst_header();
      convert_s3m_intstrument();
    } else {
      generate_blank_stm_instrument();
    }

    fwrite(stm_sample_header, sizeof(u8), sizeof(stm_sample_header), STMfile);
  }

  convert_song_orders(order_count);
  fwrite(stm_order_list, sizeof(u8), sizeof(stm_order_list), STMfile);

  return 0;
}

int convert_s3m_to_stx(FILE *S3Mfile, FILE *STXfile) {
  (void)S3Mfile; (void)STXfile;
  /* TODO */
  return 0;
}

int main(int argc, char *argv[]) {
  int return_value = EXIT_SUCCESS;
  FILE *outfile;
  FILE *infile;

  if (argc != 3) {
    printf("usage: %s <input.s3m> <output.stm>\n", argv[0]);
    return_value = EXIT_FAILURE;
    goto returndasvalue;
  }

  infile = fopen(argv[1], "rb");
  outfile = fopen(argv[2], "wb");

  if (!infile || !outfile) {
      return_value = EXIT_FAILURE;
      perror("Failed to open file");
      goto closefiledescriptors;
  }

  /* TODO: check if the user used an
   * STM or STX extension on the output name and
   * switch operation to STX conversion if they did */
  convert_s3m_to_stm(infile, outfile);

  closefiledescriptors:
  fclose(infile);
  fclose(outfile);

  returndasvalue:
  return return_value;
}
