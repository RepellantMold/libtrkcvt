#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "sample.h"

void convert_unsigned_to_signed(char* pcm, usize length) {
  usize i = 0;

  if (!length || !pcm) return;

  for (; i < length; i++) {
    pcm[i] = pcm[i] ^ 128;
  }
}

usize calculate_sample_padding(size_t sample_size) {
  return sample_size % 16;
}