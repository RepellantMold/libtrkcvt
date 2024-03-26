#ifndef __SAMPLE_H
#define __SAMPLE_H
#include "ext.h"

void convert_unsigned_to_signed(char* pcm, usize length);
usize calculate_sample_padding(size_t sample_size);

#endif
