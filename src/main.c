/*
 * Screamverter
 * Made by RepellantMold/cs127 under ISC license
 *
 * Written in C99
 */

#define _CRT_SECURE_NO_WARNINGS
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"

#include "main.h"

#include "conv.h"


u8 original_order_count = 0, order_count = 0, sample_count = 0, pattern_count = 0;

FOC_Context main_context;

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
  if (main_context.verbose_mode)
    puts(msg);
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

// S3M

u8 s3m_song_header[96];
u8 s3m_inst_header[80];

u8 s3m_order_array[S3M_ORDER_LIST_SIZE] = {S3M_ORDER_END};
u16 s3m_inst_pointers[S3M_MAXSMP] = {0};
u16 s3m_pat_pointers[S3M_MAXPAT] = {0};
u32 s3m_pcm_pointers[S3M_MAXSMP] = {0};
u16 s3m_pcm_lens[S3M_MAXSMP] = {0};
u16 s3m_cwtv;


int check_valid_s3m(FILE* S3Mfile);

int convert_s3m_to_stm(FOC_Context* context);
int convert_s3m_to_stx(FOC_Context* context);

int main(int argc, char* argv[]) {
  int i = 0, return_value = FOC_SUCCESS;
  usize conversion_type = FOC_S3MTOSTM;
  FILE *outfile = NULL, *infile = NULL;

  if (argc < 2) {
  print_da_help:
    print_help();
    return FOC_NO_FILENAMES;
  }

  for (i = 1; i < argc; i++) {
    if (!(strcmp(argv[i], "-v")) || !(strcmp(argv[i], "--verbose"))) {
      main_context.verbose_mode = true;
    }

    else if (!(strcmp(argv[i], "-s")) || !(strcmp(argv[i], "--sanitize"))) {
      main_context.sanitize_sample_names = true;
    }

    else if (!(strcmp(argv[i], "-h")) || !(strcmp(argv[i], "--help"))) {
      goto print_da_help;
    }

    else if (!(strcmp(argv[i], "-m")) || !(strcmp(argv[i], "--memory"))) {
      main_context.handle_effect_memory = true;
    }

    else if (!(strcmp(argv[i], "-stm"))) {
      conversion_type = FOC_S3MTOSTM;
    }

    else if (!(strcmp(argv[i], "-stx"))) {
      conversion_type = FOC_S3MTOSTX;
    }

    else if (!infile) {
      infile = fopen(argv[i], "rb");
    } else if (!outfile) {
      outfile = fopen(argv[i], "wb");
    }
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
    case FOC_S3MTOSTM: return_value |= convert_s3m_to_stm(&main_context); break;
    case FOC_S3MTOSTX: return_value |= convert_s3m_to_stx(&main_context); break;
  }

closefiledescriptors:
  fclose(infile);
  fclose(outfile);

  return return_value;
}

int check_valid_s3m(FILE* S3Mfile) {
  char scrm[4] = {0};

  if (!S3Mfile)
    return FOC_MALFORMED_FILE;

  (void)!fseek(S3Mfile, 44, SEEK_SET);

  (void)!fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4)) {
    eprintf("This is not an S3M file!");
    return FOC_NOT_S3M_FILE;
  }

  rewind(S3Mfile);
  return FOC_SUCCESS;
}
