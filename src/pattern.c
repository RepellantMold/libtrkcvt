#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

void convert_effect(u8 effect, u8 parameter) {
  u8 hinib = parameter >> 4;
  u8 lownib = parameter & 0x0F;
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
      if (hinib == 0xF || lownib == 0xF)
        puts("WARNING: there's no fine volume slides!");
      else if (hinib != 0 && lownib != 0)
        printf("WARNING: both x and y specified (not allowed!); x = %d, y = %d, y will take priority!", hinib, lownib);
      goto noeffectmemory;
      break;

    /* porta up/down */
    case 6:
    case 5:
      if (parameter >= 0xE0)
        puts("WARNING: there's no fine/extra-fine porta up/down!");
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
void parse_s3m_pattern(FILE* file, usize position) {
  u16 pattern_size = 0;
  char* buffer;
  u8 c = 0, r = 0, cv = 0;

  fseek(file, position, SEEK_SET);

  fread(&pattern_size, sizeof(short int), 1, file);
  buffer = malloc(pattern_size);

  fread(buffer, sizeof(char), pattern_size, file);

  loop {
    cv = *(buffer++);
    if(!cv) {r++; break;}
    c = (cv & 15);

    s3m_unpacked_pattern[r][c][0] = (cv & 0x20) ? *(buffer++) : 0xFF;
    s3m_unpacked_pattern[r][c][1] = (cv & 0x20) ? *(buffer++) : 0x00;
    s3m_unpacked_pattern[r][c][2] = (cv & 0x40) ? *(buffer++) : 0xFF;
    s3m_unpacked_pattern[r][c][3] = (cv & 0x80) ? *(buffer++) : 0x00;
    s3m_unpacked_pattern[r][c][4] = (cv & 0x80) ? *(buffer++) : 0x00;
  }
}