#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "struct.h"

#include "sample.h"

int dump_sample_data(FILE* file, usize position, internal_sample_t* context) {
  if (!file || !context)
    return FOC_MALFORMED_BUFFER;

  (void)!fseek(file, (long)position, SEEK_SET);

  if (fread(context->pcm, sizeof(u8), context->length, file) != context->length)
    return FOC_SAMPLE_FAIL;

  return FOC_SUCCESS;
}

void pcm_swap_sign(internal_sample_t* context) {
  register usize i = 0;

  if (!context)
    return;

  if (!context->pcm || !context->length)
    return;

  do {
    context->pcm[i] ^= 128;
  } while (i++ < context->length);
}

usize calculate_sample_padding(internal_sample_t context) { return 16 - (context.length & 15); }
