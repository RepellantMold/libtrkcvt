#include <stdio.h>

#include "envcheck.h"
#include "ext.h"

// a helper from https://github.com/viiri/st2play!
u16 fgetw(FILE* fp) {
  u8 data[2];

  data[0] = (u8)fgetc(fp);
  data[1] = (u8)fgetc(fp);

  return (data[1] << 8) | data[0];
}