/*
 * Screamverter
 * Made by RepellantMold/cs127 under ISC license
 *
 * Written in C99
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "header.c"
#include "parapnt.c"
#include "sample.c"
#include "pattern.c"

enum FOC_ConversionMode
{
  FOC_S3MTOSTM        = 0x00,
  FOC_S3MTOSTX        = 0x01
};

/* a helper from https://github.com/viiri/st2play! */
u16 fgetw(FILE *fp)
{
    u8 data[2];

    data[0] = (u8)fgetc(fp);
    data[1] = (u8)fgetc(fp);

    return (data[1] << 8) | data[0];
}

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

void optional_printf(const char* format, ...) {
  if (main_context.verbose_mode) {
    va_list ap;

    va_start(ap, format);
    vprintf(format, ap);
    va_end(ap);
  }
}

void optional_puts(const char* msg) {
  if (main_context.verbose_mode) {
    puts(msg);
  }
}

void warning_puts(const char* msg) {
  printf("WARNING: ");
  puts(msg);
}

void warning_printf(const char* format, ...) {
  va_list ap;

  printf("WARNING: ");

  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
}

void print_help(void) {
  puts("Usage: screamverter [options] <inputfile> <outputfile>");
  puts("(C) RepellantMold/cs127, 2024");
  puts("Licensed under ISC");
  puts("");
  puts("Options:");
  puts("  -h, --help       Print this help and exit");
  puts("  -v, --verbose    Enable extremely verbose output");
  puts("  -s, --sanitize   Sanitize sample names during conversion, useful for saving them on DOS");
  puts("  -m, --memory     Handle effects calling effect memory (experimental)");
  puts("  -stm             Convert the S3M to STM (default)");
  puts("  -stx             Convert the S3M to STX (unfinished)");
}

void handle_sample_headers_s3mtostm(FOC_Context* context, usize sample_count);
int handle_pcm_s3mtostm(FOC_Context* context, usize sample_count);
void handle_pcm_parapointer_s3mtostm(FOC_Context* context, usize i);
void handle_patterns_s3mtostm(FOC_Context* context, usize pattern_count);

int check_valid_s3m(FILE *S3Mfile);


int convert_s3m_to_stm(FOC_Context* context);
int convert_s3m_to_stx(FOC_Context* context);

int main(int argc, char *argv[]) {
  int i = 0, return_value = FOC_SUCCESS;
  usize conversion_type = FOC_S3MTOSTM;
  FILE *outfile = NULL, *infile = NULL;

  if (argc < 2) {
    print_da_help:
    print_help();
    return FOC_NO_INPUT;
  }

  for(i = 1; i < argc; i++) {
    if (!(strcmp(argv[i], "-v")) || !(strcmp(argv[i], "--verbose")))
      main_context.verbose_mode = true;

    else if (!(strcmp(argv[i], "-s")) || !(strcmp(argv[i], "--sanitize")))
      main_context.sanitize_sample_names = true;

    else if (!(strcmp(argv[i], "-h")) || !(strcmp(argv[i], "--help")))
      goto print_da_help;

    else if (!(strcmp(argv[i], "-m")) || !(strcmp(argv[i], "--memory")))
      main_context.handle_effect_memory = true;

    else if (!(strcmp(argv[i], "-stm")))
      conversion_type = FOC_S3MTOSTM;

    else if (!(strcmp(argv[i], "-stx")))
      conversion_type = FOC_S3MTOSTX;

    else if (!infile)
      infile = fopen(argv[i], "rb");
    else if (!outfile)
      outfile = fopen(argv[i], "wb");
  }

  if (!infile || !outfile) {
      return_value |= FOC_OPEN_FAILURE;
      perror("Failed to open file");
      goto closefiledescriptors;
  }

  main_context.infile = infile;
  main_context.outfile = outfile;
  main_context.conversion_type = conversion_type;

  switch (conversion_type) {
    case FOC_S3MTOSTM:
      return_value |= convert_s3m_to_stm(&main_context);
      break;
    case FOC_S3MTOSTX:
      return_value |= convert_s3m_to_stx(&main_context);
      break;
  }

  closefiledescriptors:
  fclose(infile);
  fclose(outfile);

  return return_value;
}

int convert_s3m_to_stm(FOC_Context* context) {
  FILE* S3Mfile = context->infile;
  FILE* STMfile = context->outfile;
  bool verbose = context->verbose_mode;

  if(!S3Mfile || !STMfile) return FOC_OPEN_FAILURE;
  if(ferror(S3Mfile) || ferror(STMfile)) return FOC_MALFORMED_FILE;
  if(check_valid_s3m(S3Mfile)) return FOC_NOT_S3M_FILE;

  (void)!fread(s3m_song_header, sizeof(u8), sizeof(s3m_song_header), S3Mfile);
  s3m_cwtv = s3m_song_header[41] << 8 | s3m_song_header[40];
  order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  if (sample_count > STM_MAXSMP) warning_printf("Sample count exceeds 31 (%u > %u), only using %u.\n", sample_count, STM_MAXSMP, STM_MAXSMP);
  pattern_count = s3m_song_header[36];
  if (pattern_count > STM_MAXPAT) warning_printf("Pattern count exceeds 63 (%u > %u), only converting %u.\n", pattern_count, STM_MAXPAT, STM_MAXPAT);
  if (verbose) show_s3m_song_header();

  check_s3m_channels();

  convert_song_header_s3mtostm();
  fwrite(stm_song_header, sizeof(u8), sizeof(stm_song_header), STMfile);

  grab_s3m_parapointers(S3Mfile);

  handle_sample_headers_s3mtostm(context, sample_count);

  convert_song_orders_s3mtostm(order_count);
  fwrite(stm_order_list, sizeof(u8), sizeof(stm_order_list), STMfile);

  handle_patterns_s3mtostm(context, pattern_count);

  if(handle_pcm_s3mtostm(context, sample_count)) {
    return FOC_SAMPLE_FAIL;
  };

    puts("Conversion done successfully!");
    return FOC_SUCCESS;
}

void handle_sample_headers_s3mtostm(FOC_Context* context, usize sample_count) {
  FILE* S3Mfile = context->infile;
  FILE* STMfile = context->outfile;
  bool verbose = context->verbose_mode;
  usize i = 0;

  for(; i < STM_MAXSMP; i++) {
    if(i >= sample_count) {
      generate_blank_stm_instrument();
      goto skiptowritingsampleheader;
    }

    if (verbose) printf("Sample %zu:\n", i);
    grab_sample_data(S3Mfile, s3m_inst_pointers[i]);
    s3m_pcm_pointers[i] = grab_s3m_pcm_pointer();
    s3m_pcm_lens[i] = grab_s3m_pcm_len();
    if (verbose) show_s3m_inst_header();
    convert_s3m_intstrument_header_s3mtostm();    

    skiptowritingsampleheader:
    fwrite(stm_sample_header, sizeof(u8), sizeof(stm_sample_header), STMfile);
  }
}

void handle_patterns_s3mtostm(FOC_Context* context, usize pattern_count) {
  FILE* S3Mfile = context->infile;
  FILE* STMfile = context->outfile;
  usize i = 0;

  for(i = 0; i < STM_MAXPAT; i++) {
    if (i >= pattern_count) break;
    printf("Converting pattern %zu...\n", i);
    parse_s3m_pattern(S3Mfile, s3m_pat_pointers[i]);
    convert_s3m_pattern_to_stm();
    fwrite(stm_pattern, sizeof(u8), sizeof(stm_pattern), STMfile);
    printf("Pattern %zu written.\n", i);
  }
}

int handle_pcm_s3mtostm(FOC_Context* context, usize sample_count) {
  FILE* S3Mfile = context->infile;
  FILE* STMfile = context->outfile;
  usize i = 0;
  u8 *stm_sample_data = NULL, *temp = NULL;
  Sample_Context sc;
  usize sample_len = 0, padding_len = 0;

  for(; i < STM_MAXSMP; i++) {
    if(i >= sample_count) break;

    sample_len = s3m_pcm_lens[i];

    if (!sample_len) continue;

    padding_len = (u16)calculate_sample_padding(sample_len);

    stm_sample_data = calloc(sample_len, sizeof(u8));
    if (!stm_sample_data) {
      eprintf("Could not allocate memory for sample data!\n");
      return FOC_ALLOC_FAIL;
    }

    sc.length = sample_len;
    sc.pcm = stm_sample_data;

    printf("Converting sample %zu...\n", i);

    if(dump_sample_data(S3Mfile, s3m_pcm_pointers[i], &sc)) return FOC_SAMPLE_FAIL;
    convert_unsigned_to_signed(&sc);

    if (!padding_len) goto dontaddpadding;

    sample_len += padding_len;

    temp = realloc(stm_sample_data, sample_len);
    if (!temp) {
      free(stm_sample_data);
      eprintf("Could not reallocate memory for sample data!\n");
      return FOC_ALLOC_FAIL;
    }
    stm_sample_data = temp;

    dontaddpadding:
    handle_pcm_parapointer_s3mtostm(context, i);

    (void)!fwrite(stm_sample_data, sizeof(u8), sample_len, STMfile);

    (void)!printf("Sample %zu written.\n", i);

    free(stm_sample_data);
  }

  return FOC_SUCCESS;
}

void handle_pcm_parapointer_s3mtostm(FOC_Context* context, usize i) {
  usize saved_pos = (usize)ftell(context->outfile);
  usize header_pos = sizeof(stm_song_header) + ((sizeof(stm_sample_header) * (i + 1)) - 18);

  stm_pcm_pointers[i] = calculate_stm_sample_parapointer();

  (void)!fseek(context->outfile, (long)header_pos, SEEK_SET);

  (void)!fwrite(&stm_pcm_pointers[i], sizeof(u8), 2, context->outfile);

  (void)!fseek(context->outfile, (long)saved_pos, SEEK_SET);
}

int check_valid_s3m(FILE *S3Mfile) {
  char scrm[4] = {0};

  if(!S3Mfile) return FOC_MALFORMED_FILE;

  (void)!fseek(S3Mfile, 44, SEEK_SET);

  (void)!fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4)) {
    eprintf("This is not an S3M file!");
    return FOC_NOT_S3M_FILE;
  }

  rewind(S3Mfile);
  return FOC_SUCCESS;
}

int convert_s3m_to_stx(FOC_Context* context) {
  FILE* S3Mfile = context->infile;
  FILE* STXfile = context->outfile;
  bool verbose = context->verbose_mode;

  warning_puts("This feature is not finished!\n");

  if(!S3Mfile || !STXfile) return FOC_OPEN_FAILURE;
  if(ferror(S3Mfile) || ferror(STXfile)) return FOC_MALFORMED_FILE;
  if(check_valid_s3m(S3Mfile)) return FOC_NOT_S3M_FILE;

  (void)!fread(s3m_song_header, sizeof(u8), sizeof(s3m_song_header), S3Mfile);
  order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  if (sample_count > STM_MAXSMP) warning_printf("Sample count exceeds %u (%u > %u), only using %u.\n", STM_MAXSMP, sample_count, STM_MAXSMP, STM_MAXSMP);
  pattern_count = s3m_song_header[36];
  if (pattern_count > STM_MAXPAT) warning_printf("Pattern count exceeds %u (%u > %u), only converting %u.\n", STM_MAXPAT, pattern_count, STM_MAXPAT, STM_MAXPAT);
  if (verbose) show_s3m_song_header();

  check_s3m_channels();

  convert_song_header_s3mtostx();
  fwrite(stx_song_header, sizeof(u8), sizeof(stx_song_header), STXfile);

  return FOC_SUCCESS;
}
