#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"
#include "main.h"
#include "pattern.h"

#define EFFBASE ('A' - 1)
#define EFF(e) (e - EFFBASE)

void check_effect(u8 effect, u8 parameter) {
  u8 hinib = parameter >> 4;
  u8 lownib = parameter & 0x0F;
  switch (effect) {
    default:
      eprintf("WARNING: unsupported effect %c!\n", EFFBASE + effect);
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
        eprintf("WARNING: both x (%hhu) and y (%hhu) specified, y will take priority!\n", hinib, lownib);
      goto noeffectmemory;
      break;

    /* porta up/down */
    case EFF('E'):
    case EFF('F'):
      if (hinib >= 0xE)
        eputs("WARNING: there's no fine/extra-fine porta up/down!");
      goto noeffectmemory;
      break;

    /* tone porta */
    case EFF('G'):
      goto noeffectmemory;
      break;

    /* vibrato */
    case EFF('H'):
      eputs("WARNING: vibrato depth is doubled compared to other trackers, attempting to make adjustment.");
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
    eputs("WARNING: there's no effect memory, this will be treated as a no-op.");
  return;
}

/* prototype function (NOT TESTED) */
void parse_s3m_pattern(FILE* file, usize position) {
  u8 c = 0, r = 0, cv = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  if(!file || !position) return;

  fseek(file, position, SEEK_SET);

  fseek(file, 2, SEEK_CUR);

  while (r++ < MAXROWS) {
    fread(&cv, sizeof(u8), 1, file);

    if(!cv) {break;}

    c = (cv & 31);

    if (cv & 0x20)
    {
      fread(&note, sizeof(u8), 1, file);
      fread(&ins, sizeof(u8), 1, file);
    } else {
      note = 0xFF;
      ins = 0x00;
    }

    if (cv & 0x40)
    {
      fread(&volume, sizeof(u8), 1, file);
    } else {
      volume = 0xFF;
    }

    if (cv & 0x80)
    {
      fread(&effect, sizeof(u8), 1, file);
      fread(&parameter, sizeof(u8), 1, file);
    } else {
      effect = 0x00;
      parameter = 0x00;
    }

    printf("r:%02u c:%02u %.2s%01u %02u %02u %c%02X\n",
            r, c, (note < 0xFE) ? notetable[note % 12] : (note == 0xFE) ? (u8*)"^^" : (u8*)"--", (note < 0xFE) ? note/12 : 0, ins, (volume <= 64) ? volume : 0, EFFBASE + effect, parameter);

    s3m_unpacked_pattern[r][c][0] = note;
    s3m_unpacked_pattern[r][c][1] = ins;
    s3m_unpacked_pattern[r][c][2] = volume;
    s3m_unpacked_pattern[r][c][3] = effect;
    s3m_unpacked_pattern[r][c][4] = parameter;
  };

}

void convert_s3m_pattern_to_stm(void) {
  usize r = 0, c = 0;
  u8 note = 0xFF, ins = 0, volume = 0xFF, effect = 0, parameter = 0;

  for(c = 0; c < STM_MAXCHN; c++) {
    for(r = 0; r < MAXROWS; r++) {
      note = s3m_unpacked_pattern[r][c][0],
      ins = s3m_unpacked_pattern[r][c][1],
      volume = s3m_unpacked_pattern[r][c][2],
      effect = s3m_unpacked_pattern[r][c][3],
      parameter = s3m_unpacked_pattern[r][c][4];

      check_effect(effect, parameter);

      stm_pattern[r][c][0] = note,
      stm_pattern[r][c][1] = (ins << 4) | (volume & 15),
      stm_pattern[r][c][2] = ((volume & 15) << 4) | (effect & 15),
      stm_pattern[r][c][3] = parameter;
    }
  }
}

void generate_blank_stm_pattern(void) {
  usize r = 0, c = 0;
  for(c = 0; c < STM_MAXCHN; c++) {
    for(r = 0; r < MAXROWS; r++) {
      stm_pattern[r][c][0] = 0xFF,
      stm_pattern[r][c][1] = 0x01,
      stm_pattern[r][c][2] = 0x80,
      stm_pattern[r][c][3] = 0x00;
    }
  }
}

/*
void convert_s3m_pattern_to_stx(unsigned char* buffer) {
  // TODO
}
*/