#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "sample.h"

void dump_sample_data(FILE* file, usize position, u8* pcm, usize length) {
	fseek(file, position, SEEK_SET);

	fread(pcm, sizeof(u8), length, file);
}

void convert_unsigned_to_signed(u8* pcm, usize length) {
  usize i = 0;

  if (!length || !pcm) return;

  while (i++ < length)
    pcm[i] ^= 128;
}

usize calculate_sample_padding(size_t sample_size) {
  return sample_size & 0xF;
}

/* the following code is from https://github.com/aeldidi/crc32/,
 * which is licensed under CC0, modified for code style */

u32 crc32_for_byte(u32 byte) {
	const u32 polynomial = 0xEDB88320L;
	u32 result = byte;
	size_t i = 0;

	for (; i < 8; i++) {
		/* IMPLEMENTATION: the code below always shifts result right by
		 * 1, but only XORs it by the polynomial if we're on the lowest
		 * bit.
		 *
		 * This is because 1 in binary is 00000001, so ANDing the
		 * result by 1 will always give 0 unless the lowest bit is set.
		 * And since XOR by zero does nothing, the other half only
		 * occurs when we're on the lowest bit.
		 *
		 * I didn't leave the above implementation in, despite being
		 * faster on my machine since it is a more complex operation
		 * which may be slower on less sophisticated processors. It can
		 * be added in in place of the loop code below.
		 */

		result = (result >> 1) ^ (result & 1) * polynomial;

		/* Here is the code I replaced with the branch I tried to
		 * remove:
		if (result & 1) {
			result = (result >> 1) ^ polynomial;
			continue;
		}
		result >>= 1;
		 */
	}
	return result;
}

u32 crc32(const unsigned char *input, usize size) {
	const u8 *current = input;
	u32 result = 0xFFFFFFFF;
	usize i = 0;

	for (; i < size; i++) {
		result ^= current[i];
		result = crc32_for_byte(result);
	}

	return ~result;
}
