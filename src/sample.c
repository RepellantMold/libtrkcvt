#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"

#include "main.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"

#include "sample.h"

int dump_sample_data(FILE* file, usize position, Sample_Context* context) {
  if (!file || !context)
    return FOC_MALFORMED_BUFFER;

  (void)!fseek(file, (long)position, SEEK_SET);

  if (fread(context->pcm, sizeof(u8), context->length, file) != context->length)
    return FOC_SAMPLE_FAIL;

  return FOC_SUCCESS;
}

void pcm_swap_sign(Sample_Context* context) {
  register usize i = 0;

  if (!context)
    return;

  if (!context->pcm || !context->length)
    return;

  do {
    context->pcm[i] ^= 128;
  } while (i++ < context->length);
}

usize calculate_sample_padding(usize sample_size) { return 16 - (sample_size & 15); }
