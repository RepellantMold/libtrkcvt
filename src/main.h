#ifndef __MAIN_H
#define __MAIN_H
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "envcheck.h"
#include "ext.h"

#include "s3m.h"
#include "stm.h"

static u8 original_order_count = 0, order_count = 0, sample_count = 0, pattern_count = 0;

typedef struct {
  FILE* infile;
  FILE* outfile;
  usize conversion_type;
  bool verbose_mode;
  bool sanitize_sample_names;
  bool handle_effect_memory;
} FOC_Context;

enum FOC_ConversionMode { FOC_S3MTOSTM = 0x00, FOC_S3MTOSTX = 0x01 };

// RM: stealing cs127's NTCheck's return values!
enum FOC_ReturnCode {
  FOC_SUCCESS = 0x00,
  FOC_OPEN_FAILURE = 0x01,
  FOC_NOT_S3M_FILE = 0x02,
  FOC_MALFORMED_FILE = 0x04,
  FOC_CONV_FAILURE = 0x08,
  FOC_ALLOC_FAIL = 0x10,
  FOC_MALFORMED_BUFFER = 0x20,
  FOC_NO_FILENAMES = 0x40,
  FOC_SAMPLE_FAIL = 0x80,
};

static FOC_Context main_context;

void eprintf(const char* format, ...);
void eputs(const char* msg);

void optional_printf(const char* format, ...);
void optional_puts(const char* msg);

void warning_puts(const char* msg);
void warning_printf(const char* format, ...);

#endif
