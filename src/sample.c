#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "sample.h"

void dump_sample_data(FILE* file, usize position, Sample_Context* context) {
  if (!file || !context) return;

  fseek(file, position, SEEK_SET);

  (void)!fread(context->pcm, sizeof(u8), context->length, file);
}

void convert_unsigned_to_signed(Sample_Context* context) {
  usize i = 0;

  if (!context || !context) return;

  while (i++ < context->length)
    context->pcm[i] ^= 128;
}

usize calculate_sample_padding(size_t sample_size) {
  return 16 - (sample_size % 16);
}

