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

void print_s3m_pattern() {
  usize c = 0, r = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  for (r = 0; r < MAXROWS; r++) {
    optional_printf("r:%02u = ", r);

    for(c = 0; c < S3M_MAXCHN >> 3; c++) {
      note = s3m_unpacked_pattern[r][c][0];
      ins = s3m_unpacked_pattern[r][c][1];
      volume = s3m_unpacked_pattern[r][c][2];
      effect = s3m_unpacked_pattern[r][c][3];
      parameter = s3m_unpacked_pattern[r][c][4];

      if (note < 0xFE)
      optional_printf("%.2s%01u", notetable[note % 12], note/12);
      else if (note == 0xFE) optional_printf("^^^");
      else optional_printf("...");

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
  }

  fputs("\n", stdout);
}

void check_effect(Pattern_Display_Context* context) {
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
      /* TODO: implement speed factor? */
      parameter <<= 4;
      break;

    /* set position */
    case EFF('B'):
      warning_pattern_puts(context, "set position does not do a pattern break, please use a pattern break alongside this if it's intended!");
      break;

    /* pattern break */
    case EFF('C'):
      if (parameter) {
        warning_pattern_puts(context, "pattern break ignores parameter!");
        parameter = 0;
      }
      break;

    /* volume slide */
    case EFF('D'):
      if (hinib == 0xF || lownib == 0xF)
        warning_pattern_puts(context, "there's no fine volume slides!");
      else if (hinib && lownib)
        warning_pattern_printf(context, "both x (%hhu) and y (%hhu) specified, y will take priority!\n", hinib, lownib);
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
      warning_pattern_puts(context, "vibrato depth is doubled compared to other trackers, attempting to make adjustment.");
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

    default:
      warning_pattern_puts(context, "unsupported effect!");
      effect = 0;
      break;
  }

  return;

  noeffectmemory:
  if (!parameter)
    warning_pattern_puts(context, "there's no effect memory with this effect, this will be treated as a no-op.");
  return;
}

/* prototype function (NOT TESTED) */
void parse_s3m_pattern(FILE* file, usize position) {
  u8 c = 0, r = 0, cv = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  if(!file || !position) return;
  if(feof(file) || ferror(file)) return;

  fseek(file, position, SEEK_SET);

  fseek(file, 2, SEEK_CUR);

  while (r < MAXROWS) {
    (void)!fread(&cv, sizeof(u8), 1, file);

    if(!cv) {r++; continue;}

    c = (cv & 31);

    if (cv & 0x20)
    {
      (void)!fread(&note, sizeof(u8), 1, file);
      (void)!fread(&ins, sizeof(u8), 1, file);
    } else {
      note = 0xFF;
      ins = 0x00;
    }

    if (cv & 0x40)
    {
      (void)!fread(&volume, sizeof(u8), 1, file);
    } else {
      volume = 0xFF;
    }

    if (cv & 0x80)
    {
      (void)!fread(&effect, sizeof(u8), 1, file);
      (void)!fread(&parameter, sizeof(u8), 1, file);
    } else {
      effect = 0x00;
      parameter = 0x00;
    }

    s3m_unpacked_pattern[r][c][0] = note;
    s3m_unpacked_pattern[r][c][1] = ins;
    s3m_unpacked_pattern[r][c][2] = volume;
    s3m_unpacked_pattern[r][c][3] = effect;
    s3m_unpacked_pattern[r][c][4] = parameter;

    if (main_context.verbose_mode) print_s3m_pattern();
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

      Pattern_Display_Context pd = {r, c, effect, parameter};

      check_effect(&pd);

      stm_pattern[r][c][0] = note,
      stm_pattern[r][c][1] = ((ins & 31) << 3) | (volume & 15),
      stm_pattern[r][c][2] = ((volume & 7) << 3) | (effect & 15),
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

