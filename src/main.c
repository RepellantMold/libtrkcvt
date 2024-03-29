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

/* RM: stealing cs127's NTCheck's return values! */
enum FOC_ReturnCode
{
    FOC_SUCCESS        = 0x00,

    FOC_OPEN_FAILURE   = 0x01,
    FOC_NOT_S3M_FILE   = 0x02,
    FOC_MALFORMED_FILE = 0x04,
    FOC_CONV_FAILURE   = 0x08,
    FOC_ALLOC_FAIL     = 0x10,

    FOC_NO_FILENAMES   = 0x40
};


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

  (void)!fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4)) {
    eprintf("This is not an S3M file!");
    return false;
  }

  rewind(S3Mfile);
  return true;
}

int convert_s3m_to_stm(FILE *S3Mfile, FILE *STMfile) {
  usize i = 0;
  u8* stm_sample_data = NULL;
  u8* temp;
  u16 sample_len = 0;
  u16 padding_len = 0;

  if(!S3Mfile || !STMfile) return FOC_OPEN_FAILURE;
  if(ferror(S3Mfile) || ferror(STMfile)) return FOC_MALFORMED_FILE;
  if(!check_valid_s3m(S3Mfile)) return FOC_NOT_S3M_FILE;

  (void)!fread(s3m_song_header, sizeof(u8), sizeof(s3m_song_header), S3Mfile);
  order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  if (sample_count > STM_MAXSMP) eprintf("WARNING: Sample count exceeds 31 (%u > %u), only using %u.\n", sample_count, STM_MAXSMP, STM_MAXSMP);
  pattern_count = s3m_song_header[36];
  if (pattern_count > STM_MAXPAT) eprintf("WARNING: Pattern count exceeds 63 (%u > %u), only converting %u.\n", pattern_count, STM_MAXPAT, STM_MAXPAT);
  show_s3m_song_header();

  check_s3m_channels();

  convert_song_header();
  fwrite(stm_song_header, sizeof(u8), sizeof(stm_song_header), STMfile);

  grab_s3m_parapointers(S3Mfile);

  for(i = 0; i < STM_MAXSMP; i++) {
    if(i < sample_count) {
      printf("Sample %u:\n", (u8)i);
      grab_sample_data(S3Mfile, s3m_inst_pointers[i]);
      s3m_pcm_pointers[i] = grab_s3m_pcm_pointer();
      s3m_pcm_lens[i] = grab_s3m_pcm_len();
      show_s3m_inst_header();
      convert_s3m_intstrument();
    } else {
      generate_blank_stm_instrument();
    }

    fwrite(stm_sample_header, sizeof(u8), sizeof(stm_sample_header), STMfile);
  }

  convert_song_orders(order_count);
  fwrite(stm_order_list, sizeof(u8), sizeof(stm_order_list), STMfile);

  for(i = 0; i < STM_MAXPAT; i++) {
    if (i < pattern_count) {
      printf("Pattern %u:\n", (u8)i);
      parse_s3m_pattern(S3Mfile, s3m_pat_pointers[i]);
      convert_s3m_pattern_to_stm();
      fwrite(stm_pattern, sizeof(u8), sizeof(stm_pattern), STMfile);
      printf("Pattern %u written.\n", (u8)i);
    } else {
      break;
    }
  }

  for(i = 0; i < STM_MAXSMP; i++) {
    if(i < sample_count) {
      sample_len = s3m_pcm_lens[i];

      if (!sample_len) continue;

      padding_len = (u16)calculate_sample_padding(sample_len);

      stm_sample_data = calloc(sample_len, sizeof(u8));
      if (!stm_sample_data) {
        eprintf("Could not allocate memory for sample data!\n");
        return FOC_ALLOC_FAIL;
      }

      printf("Converting sample %u...\n", (u8)i);

      dump_sample_data(S3Mfile, s3m_pcm_pointers[i], stm_sample_data, sample_len);
      convert_unsigned_to_signed(stm_sample_data, sample_len);

      if (padding_len) {
        sample_len += padding_len;

        temp = realloc(stm_sample_data, sample_len);
        if (!temp) {
          free(stm_sample_data);
          eprintf("Could not reallocate memory for sample data!\n");
          return FOC_ALLOC_FAIL;
        }
        stm_sample_data = temp;
      }

      fwrite(stm_sample_data, sizeof(u8), sample_len, STMfile);

      printf("Sample %u written.\n", (u8)i);

      free(stm_sample_data);
    }
  }

  return FOC_SUCCESS;
}

int convert_s3m_to_stx(FILE *S3Mfile, FILE *STXfile) {
  (void)S3Mfile; (void)STXfile;
  /* TODO */
  return FOC_SUCCESS;
}

int main(int argc, char *argv[]) {
  int return_value = FOC_SUCCESS;
  FILE *outfile;
  FILE *infile;

  if (argc != 3) {
    printf("usage: %s <input.s3m> <output.stm>\n", argv[0]);
    return_value = FOC_NO_FILENAMES;
    goto returndasvalue;
  }

  infile = fopen(argv[1], "rb");
  outfile = fopen(argv[2], "wb");

  if (!infile || !outfile) {
      return_value |= FOC_OPEN_FAILURE;
      perror("Failed to open file");
      goto closefiledescriptors;
  }

  /* TODO: check if the user used an
   * STM or STX extension on the output name and
   * switch operation to STX conversion if they did */
  if(convert_s3m_to_stm(infile, outfile)) {
    eprintf("Failed to convert S3M to STM\n");
    return_value = FOC_CONV_FAILURE;
  };

  closefiledescriptors:
  fclose(infile);
  fclose(outfile);

  returndasvalue:
  return return_value;
}
