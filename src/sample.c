#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "envcheck.h"
#include "s3m.h"
#include "stm.h"

void convert_unsigned_to_signed(char *data, int length) {
  size_t i = 0;
  for (; i < length; i++) {
    data[i] = (data[i] + 128) & 0xFF;
  }
}