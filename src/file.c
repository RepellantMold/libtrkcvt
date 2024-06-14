#include <stdio.h>

#include "envcheck.h"
#include "ext.h"

u8 fgetb(FILE* fp) {
  return (u8)fgetc(fp);
}

// a helper from https://github.com/viiri/st2play
u16 fgetw(FILE* fp) {
  u8 data[2];

  data[0] = fgetb(fp);
  data[1] = fgetb(fp);

  return (data[1] << 8) | data[0];
}

// another helper from https://github.com/viiri/st2play
u32 fgetl(FILE *fp)
{
	u8 data[4];

	data[0] = fgetb(fp);
	data[1] = fgetb(fp);
	data[2] = fgetb(fp);
	data[3] = fgetb(fp);

	return (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
}

u16 fputw(u16 value, FILE* fp) {
  u8 data[2];

  data[0] = value & 0xFF;
  data[1] = value >> 8;

  return fwrite(data, sizeof(u8), 2, fp);
}

u32 fputl(u32 value, FILE* fp) {
  u8 data[4];

  data[0] = value & 0xFF;
  data[1] = (value >> 8) & 0xFF;
  data[2] = (value >> 16) & 0xFF;
  data[3] = value >> 24;

  return fwrite(data, sizeof(u8), 4, fp);
}
