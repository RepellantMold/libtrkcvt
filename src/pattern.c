#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "s3m.h"
#include "stm.h"

void convert_effect(unsigned char effect, unsigned char parameter) {
  switch (effect) {
    /* no effect */
    case 0:
      break;

    /* set speed */
    case 1:
      /* TODO: implement speed factor? */
      parameter <<= 4;
      break;

    /* set position */
    case 2:
      puts("WARNING: set position does not do a pattern break, please use a pattern break alongside this if it's intended!");
      break;

    /* pattern break */
    case 3:
      if (parameter > 0) {
        puts("WARNING: pattern break ignores parameter!");
        parameter = 0;
      }
      break;
  }

  return;

  noeffectmemory: if (!parameter) puts("WARNING: there's no effect memory, this will be treated as a no-op.");
}