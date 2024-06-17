#include "crc.h"

 /* 
  * The following replicates the "Amiga conversion file extensions"
  * section from ST2's TECH.DOC, beware though..
  * the wording is strange!
  */
u32 crc_sum(const unsigned char* input, usize size) {
  const u8* current = input;
  register u32 result = 0;
  register usize i = 0;

  for (; i < size; i++) {
    result += current[i];
  }

  return (result) + (result & 511);
}
