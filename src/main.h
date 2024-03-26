#ifndef __MAIN_H
#define __MAIN_H
#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

u8 order_count;
u8 sample_count;
u8 pattern_count;

int check_valid_s3m(FILE *S3Mfile) {
  char scrm[4] = {0};

  fseek(S3Mfile, 44, SEEK_SET);

  fread(scrm, sizeof(char), 4, S3Mfile);

  if (memcmp(scrm, "SCRM", 4) != 0) {
    fprintf(stderr, "Not an S3M file!");
    return 1;
  }

  rewind(S3Mfile);
  return 0;
}

#endif
