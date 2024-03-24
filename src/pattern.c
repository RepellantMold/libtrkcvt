#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

void convert_effect(u8 effect, u8 parameter) {
  switch (effect) {
    default:
      puts("WARNING: unknown effect!");
      break;

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

    /* volume slide */
    case 4:
      if ((parameter & 0xF0) || (parameter & 0x0F)) 
        puts("WARNING: there's no fine volume slides!");
      goto noeffectmemory;
      break;

    /* porta up/down */
    case 6:
    case 5:
      if (parameter >= 0xE0)
        puts("WARNING: there's no fine porta up/down!");
      goto noeffectmemory;
      break;

    /* tone porta */
    case 7:
      goto noeffectmemory;
      break;
    
    /* vibrato */
    case 8:
      puts("WARNING: vibrato depth is doubled compared to other trackers.");
      goto noeffectmemory;
      break;

    /* tremor */
    case 9:
      goto noeffectmemory;
      break;

    /* arpeggio */
    case 10:
      goto noeffectmemory;
      break;
  }

  return;

  noeffectmemory:
  if (!parameter)
    puts("WARNING: there's no effect memory, this will be treated as a no-op.");
  return;
}

/* prototype function (NOT TESTED) */
char* parse_s3m_pattern(FILE* file, usize position) {
  usize pattern_size;
  char* buffer;
  u8 c = 1, r = 1, cv = 0;
  char* p[S3M_UNPACKED_PATTERN_SIZE];

  fseek(file, position, SEEK_SET);

  fread(pattern_size, sizeof(short int), 1, file);
  buffer = malloc(pattern_size);

  fread(buffer, sizeof(char), pattern_size, file);

  loop {
    cv = *(buffer++);
    if(!cv) {r++; break;}
    c = (cv & 15) + 1;

    p[c*r*1] = (cv & 0x20) ? *(buffer++) : 0xFF;
    p[c*r*2] = (cv & 0x20) ? *(buffer++) : 0x00;
    p[c*r*3] = (cv & 0x40) ? *(buffer++) : 0xFF;
    p[c*r*4] = (cv & 0x80) ? *(buffer++) : 0x00;
    p[c*r*5] = (cv & 0x80) ? *(buffer++) : 0x00;
  }

  return p;
}