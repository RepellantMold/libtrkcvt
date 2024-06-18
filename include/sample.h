#ifndef __SAMPLE_H
#define __SAMPLE_H
#include "ext.h"

typedef struct {
  u8* pcm;
  usize length;
} internal_sample_t;

int dump_sample_data(FILE* file, usize position, internal_sample_t* context);
void pcm_swap_sign(internal_sample_t* context);
usize calculate_sample_padding(size_t sample_size);

#endif
