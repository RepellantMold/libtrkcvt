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
  if (!file || !pcm) return;

  fseek(file, position, SEEK_SET);

  (void)!fread(pcm, sizeof(u8), length, file);
}

void convert_unsigned_to_signed(u8* pcm, usize length) {
  usize i = 0;

  if (!length || !pcm) return;

  while (i++ < length)
    pcm[i] ^= 128;
}

usize calculate_sample_padding(size_t sample_size) {
  return 16 - (sample_size % 16);
}

