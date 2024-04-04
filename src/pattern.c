#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"
#include "main.h"
#include "pattern.h"

#define EFFBASE ('A' - 1)
#define EFF(e) (e - EFFBASE)

void warning_pattern_puts(Pattern_Display_Context* context, const char* msg) {
  printf("WARNING (row %02u/channel %02u, effect %c): ", context->row, context->channel, EFFBASE + context->effect);
  puts(msg);
}

void warning_pattern_printf(Pattern_Display_Context* context, const char* format, ...) {
  va_list ap;

  printf("WARNING (row %02u/channel %02u, effect %c): ", context->row, context->channel, EFFBASE + context->effect);

  va_start(ap, format);
  vprintf(format, ap);
  va_end(ap);
}

void print_s3m_pattern(void) {
  usize c = 0, r = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  do {
    optional_printf("r:%02u = ", r);

    for(c = 0; c < 4; c++) {
      note = s3m_unpacked_pattern[r][c][0];
      ins = s3m_unpacked_pattern[r][c][1];
      volume = s3m_unpacked_pattern[r][c][2];
      effect = s3m_unpacked_pattern[r][c][3];
      parameter = s3m_unpacked_pattern[r][c][4];

      if (note < 0xFE)
      optional_printf("%.2s%01u", notetable[note & 0x0F], (note>>4) + 1);
      else if (note == 0xFE) optional_printf("^^^");
      else if (note == 0xFF) optional_printf("...");

      if (ins) optional_printf(" %02u ", ins);
      else optional_printf(" .. ");

      if (volume <= 64) 
      optional_printf("%02u ", volume);
      else optional_printf(".. ");

      if (effect)
      optional_printf("%c%02X ", EFFBASE + effect, parameter);
      else
      optional_printf("... ");
    }
    fputs("\n", stdout);
  } while (r++ < MAXROWS-1);

  fputs("\n", stdout);
}

int check_effect(Pattern_Display_Context* context) {
  u8 effect = context->effect;
  u8 parameter = context->parameter;

  u8 hinib = parameter >> 4;
  u8 lownib = parameter & 0x0F;
  switch (effect) {
    /* no effect */
    case 0:
      break;

    /* set speed */
    case EFF('A'):
      break;

    /* set position */
    case EFF('B'):
      if (context->row < 63)
        warning_pattern_puts(context, "set position does not do a pattern break, please use a pattern break alongside this if it's intended!");
      break;

    /* pattern break */
    case EFF('C'):
      if (parameter)
        warning_pattern_puts(context, "pattern break ignores parameter!");
      break;

    /* volume slide */
    case EFF('D'):
      if ((hinib == 0xF && lownib > 0) || (lownib == 0xF && hinib > 0))
        warning_pattern_puts(context, "there's no fine volume slides!");
      else if (hinib && lownib)
        warning_pattern_printf(context, "both x (%1X) and y (%1X) specified, y will take priority!\n", hinib, lownib);
      goto noeffectmemory;
      break;

    /* porta up/down */
    case EFF('E'):
    case EFF('F'):
      if (hinib >= 0xE)
        warning_pattern_puts(context, "there's no fine/extra-fine porta up/down!");
      goto noeffectmemory;
      break;

    /* tone porta */
    case EFF('G'):
      goto noeffectmemory;
      break;

    /* vibrato */
    case EFF('H'):
      if (!main_context.handle_effect_memory && !parameter) {
        warning_pattern_puts(context, "there's no effect memory with this effect, this will be treated as a no-op.");
        break;
      }
      warning_pattern_puts(context, "vibrato depth is doubled compared to other trackers!");
      break;

    /* tremor */
    case EFF('I'):
      if (s3m_cwtv >= 0x1300 && s3m_cwtv < 0x1320)
        if (!parameter)
          warning_pattern_puts(context, "there's no effect memory with this effect, this will be treated as a really fast tremor.");
      break;

    /* arpeggio */
    case EFF('J'):
      goto noeffectmemory;
      break;

    default:
      warning_pattern_puts(context, "unsupported effect!");
      break;
  }

  return effect;

  noeffectmemory:
  if (!main_context.handle_effect_memory && !parameter)
    warning_pattern_puts(context, "there's no effect memory with this effect, this will be treated as a no-op.");
  return effect;
}

/* prototype function (NOT TESTED) */
void parse_s3m_pattern(FILE* file, usize position) {
  u8 c = 0, r = 0, cv = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  if(!file || !position) return;
  if(feof(file) || ferror(file)) return;

  fseek(file, (long)position, SEEK_SET);

  fseek(file, 2, SEEK_CUR);

  flush_s3m_pattern_array();

  while (r < MAXROWS) {
    cv = (u8)fgetc(file);

    if(!cv) {r++; continue;}

    c = (cv & 31);

    if (cv & 0x20)
    {
      note = (u8)fgetc(file);
      ins = (u8)fgetc(file);
    } else {
      note = 0xFF;
      ins = 0x00;
    }

    if (cv & 0x40)
    {
      volume = (u8)fgetc(file);
    } else {
      volume = 0xFF;
    }

    if (cv & 0x80)
    {
      effect = (u8)fgetc(file);
      parameter = (u8)fgetc(file);
    } else {
      effect = 0x00;
      parameter = 0x00;
    }

    s3m_unpacked_pattern[r][c][0] = note;
    s3m_unpacked_pattern[r][c][1] = ins;
    s3m_unpacked_pattern[r][c][2] = volume;
    s3m_unpacked_pattern[r][c][3] = effect;
    s3m_unpacked_pattern[r][c][4] = parameter;
  };

  if (main_context.verbose_mode) print_s3m_pattern();

}

int check_for_free_channel(usize r) {
  usize i = 0;

  for (; i < STM_MAXCHN; i++) {
    if (!s3m_unpacked_pattern[r][i][3]) return i;
  }

  return -1;
}

u8 search_for_last_nonzero_param(usize startingrow, usize c, usize effect) {
  usize i = startingrow;

  optional_printf("searching for last nonzero param for %c starting at row %02u and channel %02u\n", effect + EFFBASE, i, c);

  while (i-- != 0) {
    optional_printf("checking row %02u\n", i);
    if (!s3m_unpacked_pattern[i][c][4] || s3m_unpacked_pattern[i][c][3] != effect) continue;
    optional_printf("param is %02X\n", s3m_unpacked_pattern[i][c][4]);
    return s3m_unpacked_pattern[i][c][4];
  }

  optional_printf("no matches found...\n");
  return 0; 
}

u8 search_for_last_nonzero_param2(usize startingrow, usize c, usize effect) {
  usize i = startingrow;

  u8 lownib = 0,
     hinib  = 0,
     param  = 0;

  optional_printf("searching for last nonzero param for %c starting at row %02u and channel %02u\n", effect + EFFBASE, i, c);

  for (i = startingrow; i != 0; i--) {
    optional_printf("checking row %02u for low nibble\n", i);

    if (!(s3m_unpacked_pattern[i][c][4] & 0x0F) || s3m_unpacked_pattern[i][c][3] != effect) continue;

    lownib = s3m_unpacked_pattern[i][c][4] & 0x0F;
  }

  for (i = startingrow; i != 0; i--) {
    optional_printf("checking row %02u for high nibble\n", i);

    if (!(s3m_unpacked_pattern[i][c][4] >> 4) || s3m_unpacked_pattern[i][c][3] != effect) continue;

    hinib  = s3m_unpacked_pattern[i][c][4] >> 4;
  }

  if (!lownib || !hinib) goto nomatches;

  param = (hinib << 4) | lownib;
  optional_printf("param (low) is %1X and param (high) is %1X, forming %02X\n", lownib, hinib, param);
  return param;

  nomatches:
  optional_printf("no matches found...\n");
  return 0; 
}

void flush_s3m_pattern_array(void) {
  usize r = 0, c = 0;
  do {
    for (c = 0; c < STM_MAXCHN; c++) {
      s3m_unpacked_pattern[r][c][0] = 0xFF;
      s3m_unpacked_pattern[r][c][1] = 0x00;
      s3m_unpacked_pattern[r][c][2] = 0xFF;
      s3m_unpacked_pattern[r][c][3] = 0x00;
      s3m_unpacked_pattern[r][c][4] = 0x00;
    }
  } while (r++ <= MAXROWS);
}

void convert_s3m_pattern_to_stm(void) {
  usize r = 0, c = 0;
  u8 note = 0xFF, ins = 0, volume = 0xFF, effect = 0, parameter = 0;
  u8 proper_octave = 0;
  /* used for correcting effects */
  u8 lownib = 0, freechn = 0, lastprm = 0;
  Pattern_Display_Context pd;

  blank_stm_pattern();

  for(c = 0; c < STM_MAXCHN; c++) {
    for(r = 0; r < MAXROWS; r++) {
      note = s3m_unpacked_pattern[r][c][0],
      ins = s3m_unpacked_pattern[r][c][1],
      volume = s3m_unpacked_pattern[r][c][2],
      effect = s3m_unpacked_pattern[r][c][3],
      parameter = s3m_unpacked_pattern[r][c][4];

      pd.row = r;
      pd.channel = c;
      pd.effect = effect;
      pd.parameter = parameter;

      lownib = parameter & 0x0F;

      switch(check_effect(&pd)) {

        /* set speed */
        case EFF('A'):
          /* TODO: implement speed factor? */
          parameter <<= 4;
          break;

        case EFF('B'):
          freechn = check_for_free_channel(r);
          if(freechn < STM_MAXCHN) {
            s3m_unpacked_pattern[r][freechn][3] = EFF('C');
          }
          break;

        /* pattern break */
        case EFF('C'):
          parameter = 0;
          break;

        /* volume slide */
        case EFF('D'):
          goto handle_effmem;
          break;

        /* porta up/down */
        case EFF('E'):
        case EFF('F'):
          goto handle_effmem;
          break;

        /* tone porta */
        case EFF('G'):
          handle_effmem:
          if (!main_context.handle_effect_memory) break;
          if (!r || parameter != 0) break;
          lastprm = search_for_last_nonzero_param(r, c, effect);
          if(lastprm != 0)
            parameter = s3m_unpacked_pattern[r][c][4] = lastprm;
          break;

        /* tremor */
        case EFF('I'):
          /* newer scream tracker 3 versions actually have memory for this effect.. */
          if (s3m_cwtv >= 0x1300 && s3m_cwtv < 0x1320) break;
          goto handle_effmem2;
          break;

        /* arpeggio */
        case EFF('J'):
          handle_effmem2:
          if (!main_context.handle_effect_memory) break;
          if (!r || ((parameter & 0x0F) || (parameter >> 4))) break;
          lastprm = search_for_last_nonzero_param2(r, c, effect);
          if(lastprm != 0)
            parameter = s3m_unpacked_pattern[r][c][4] = lastprm;
          break;

        /* vibrato */
        case EFF('H'):
          if((lownib >> 1) != 0) {
            if(!(s3m_song_header[38] & S3M_ST2VIB)) {
              optional_printf("adjusting vibrato depth from %u to %u.\n", lownib, lownib >> 1);
              lownib >>= 1;
              optional_puts("adjustment successful!\n");
            }
          } else if (lownib != 0) optional_printf("adjustment failed... depth %u turned into %u. this will not be adjusted!\n", lownib, lownib >> 1);
          goto handle_effmem2;
          break;

        default:
          effect = 0;
          break;
      };

      if (note < 0xFE) {
        proper_octave = (note >> 4) - 2;
        note = (proper_octave << 4) | (note & 0x0F);
      }

      if (volume > 64) volume = 65;

      stm_pattern[r][c][0] = note,
      stm_pattern[r][c][1] = ((ins & 31) << 3) | (volume & 7),
      stm_pattern[r][c][2] = ((volume & 0x78) << 1) | (effect & 15),
      stm_pattern[r][c][3] = parameter;
    }
  }
}

void blank_stm_pattern(void) {
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

