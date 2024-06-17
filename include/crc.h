#ifndef __CRC32_H
#define __CRC32_H
#include "envcheck.h"
#include "ext.h"

u32 crc_sum(const unsigned char* input, usize size);

#endif
