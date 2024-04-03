#ifndef __MAIN_H
#define __MAIN_H
#include <stdlib.h>
#include <stdio.h>
#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

u8 order_count = 0;
u8 sample_count = 0;
u8 pattern_count = 0;

typedef struct {
  FILE* infile;
  FILE* outfile;
  usize conversion_type;
  bool verbose_mode;
  bool sanitize_sample_names;
} FOC_Context;

FOC_Context main_context;

void eprintf(const char* format, ...);
void eputs(const char* msg);

void optional_printf(const char* format, ...);
void optional_puts(const char* msg);

void warning_puts(const char* msg);
void warning_printf(const char* format, ...);

static u16 fgetw(FILE *fp);

#endif
