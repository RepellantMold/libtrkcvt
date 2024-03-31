#ifndef __SAMPLE_H
#define __SAMPLE_H
#include "ext.h"

typedef struct {
  u8* pcm;
  usize length;
} Sample_Context;

void convert_unsigned_to_signed(u8* pcm, usize length);
usize calculate_sample_padding(size_t sample_size);

#endif
