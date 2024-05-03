#ifndef __SAMPLE_H
#define __SAMPLE_H
#include "ext.h"

typedef struct {
  u8* pcm;
  usize length;
} Sample_Context;

int dump_sample_data(FILE* file, usize position, Sample_Context* context);
void convert_unsigned_to_signed(Sample_Context* context);
usize calculate_sample_padding(size_t sample_size);

#endif
