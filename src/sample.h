#ifndef __SAMPLE_H
#define __SAMPLE_H
#include "ext.h"

void convert_unsigned_to_signed(u8* pcm, usize length);
usize calculate_sample_padding(size_t sample_size);

u32 crc32_for_byte(u32 byte);
u32 crc32(const unsigned char *input, usize size);

#endif
