#include "crc.h"

/* the following code is from https://github.com/aeldidi/crc32/,
 * which is licensed under CC0, modified for code style */

u32 crc32_for_byte(u32 byte) {
  const u32 polynomial = 0xEDB88320L;
  u32 result = byte;
  usize i = 0;

  for (; i < 8; i++) result = (result >> 1) ^ (result & 1) * polynomial;

  return result;
}

u32 crc32(const unsigned char* input, usize size) {
  const u8* current = input;
  u32 result = 0xFFFFFFFF;
  usize i = 0;

  for (; i < size; i++) {
    result ^= current[i];
    result = crc32_for_byte(result);
  }

  return ~result;
}
