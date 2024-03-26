#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

#include "pattern.h"

#define EFFBASE ('A' - 1)
#define EFF(e) (e - EFFBASE)

void check_effect(u8 effect, u8 parameter) {
  u8 hinib = parameter >> 4;
  u8 lownib = parameter & 0x0F;
  switch (effect) {
    default:
      printf("WARNING: unsupported effect %c!", EFFBASE + effect);
      effect = 0;
      break;

    /* no effect */
    case 0:
      break;

    /* set speed */
    case EFF('A'):
      /* TODO: implement speed factor? */
      parameter <<= 4;
      break;

    /* set position */
    case EFF('B'):
      eputs("WARNING: set position does not do a pattern break, please use a pattern break alongside this if it's intended!");
      break;

    /* pattern break */
    case EFF('C'):
      if (parameter) {
        eputs("WARNING: pattern break ignores parameter!");
        parameter = 0;
      }
      break;

    /* volume slide */
    case EFF('D'):
      if (hinib == 0xF || lownib == 0xF)
        eputs("WARNING: there's no fine volume slides!");
      else if (hinib && lownib)
        eprintf("WARNING: both x and y specified (not allowed!); x = %hhu, y = %hhu, y will take priority!\n", hinib, lownib);
      goto noeffectmemory;
      break;

    /* porta up/down */
    case EFF('E'):
    case EFF('F'):
      if (parameter >= 0xE0)
        eputs("WARNING: there's no fine/extra-fine porta up/down!");
      goto noeffectmemory;
      break;

    /* tone porta */
    case EFF('G'):
      goto noeffectmemory;
      break;

    /* vibrato */
    case EFF('H'):
      puts("WARNING: vibrato depth is doubled compared to other trackers.");
      if((lownib >> 1) != 0)
        if(!(s3m_song_header[38] & S3M_ST2VIB))
          lownib >>= 1;

      goto noeffectmemory;
      break;

    /* tremor */
    case EFF('I'):
      goto noeffectmemory;
      break;

    /* arpeggio */
    case EFF('J'):
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

  if(!file) return;

  fseek(file, position, SEEK_SET);

  fread(&pattern_size, sizeof(u16), 1, file);
  buffer = malloc(pattern_size);

  fread(buffer, sizeof(char), pattern_size, file);

  while (r < 64) {
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

void convert_s3m_pattern_to_stm(void) {
  usize r = 0, c = 0;
  u8 note = 0xFF, ins = 0, volume = 0xFF, effect = 0, parameter = 0;
  
  for(c = 0; c < 4; c++) {
    for(r = 0; r < 64; r++) {
      note = s3m_unpacked_pattern[r][c][0],
      ins = s3m_unpacked_pattern[r][c][1],
      volume = s3m_unpacked_pattern[r][c][2],
      effect = s3m_unpacked_pattern[r][c][3],
      parameter = s3m_unpacked_pattern[r][c][4];

      check_effect(effect, parameter);

      stm_pattern[r][c][0] = note;
      stm_pattern[r][c][1] = (ins << 4) | (volume & 15);
      stm_pattern[r][c][2] = ((volume & 7) << 3) | (effect & 15);
      stm_pattern[r][c][3] = parameter;
    }
  }
}