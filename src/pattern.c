#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "envcheck.h"
#include "ext.h"

#include "file.h"
#include "log.h"
#include "main.h"
#include "pattern.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"

static s3mevent_t s3m_unpacked_pattern[64][32] = {{{0xFF, 0x00, 0xFF, 0x00, 0x00}}};

static const u8 notetable[12][2] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

void print_warning_pattern(Pattern_Context* context, const char* format, ...) {
  va_list ap;
  const u8 effect = EFFBASE + context->effect, row = context->row, channel = context->channel;
  char buffer[192];

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);

  if (context->effect)
    printf("WARNING (row %02u/channel %02u, effect %c): %s\n", row, channel, effect, buffer);
  else
    printf("WARNING (row %02u/channel %02u): %s\n", row, channel, buffer);
}

void print_s3m_row(usize row) {
  usize channel = 0;
  u8 note, ins, volume, effect, parameter;

  do {
    note = s3m_unpacked_pattern[row][channel].note, ins = s3m_unpacked_pattern[row][channel].ins,
    volume = s3m_unpacked_pattern[row][channel].vol, effect = s3m_unpacked_pattern[row][channel].eff,
    parameter = s3m_unpacked_pattern[row][channel].prm;

    if (note < 0xFE)
      printf("%.2s%01u", notetable[note & 0x0F], (note >> 4) + 1);
    else if (note == 0xFE)
      printf("^^^");
    else if (note == 0xFF)
      printf("...");

    if (ins)
      printf(" %02u ", ins);
    else
      printf(" .. ");

    if (volume <= 64)
      printf("%02u ", volume);
    else
      printf(".. ");

    if (effect)
      printf("%c%02X ", EFFBASE + effect, parameter);
    else
      printf("... ");
  } while (++channel < STM_MAXCHN);

  fputs("\n", stdout);
}

void print_s3m_pattern(void) {
  usize row = 0;

  for (; row < MAXROWS; ++row) {
    print_diagnostic("r:%02u = ", row);

    print_s3m_row(row);
  }

  fputs("\n", stdout);
}

int check_effect(Pattern_Context* context) {
  const u8 effect = context->effect, parameter = context->parameter;
  const u8 hinib = parameter >> 4, lownib = parameter & 0x0F;

  switch (effect) {
    case EFF_NO_EFFECT: break;

    case EFF_SET_TEMPO:
      if (hinib)
        print_warning_pattern(context, "you can only have up to $F ticks per row, found $%02X!", parameter);
      break;

    case EFF_SET_POSITION:
      if (context->row < 63) {
        print_warning_pattern(
            context,
            "set position does not do a pattern break, please use a pattern break alongside this if it's intended!");
      }
      break;

    case EFF_PATTERN_BREAK:
      if (parameter)
        print_warning_pattern(context, "pattern break ignores parameter!");
      break;

    case EFF_VOLUME_SLIDE:
      if ((hinib == 0xF && lownib > 0) || (lownib == 0xF && hinib > 0))
        print_warning_pattern(context, "there's no fine volume slides!");
      else if (hinib && lownib)
        print_warning_pattern(context, "both x (%1X) and y (%1X) specified, y will take priority!", hinib, lownib);

      goto noeffectmemory;
      break;

    case EFF_PORTA_DOWN:
    case EFF_PORTA_UP:
      if (hinib >= 0xE)
        print_warning_pattern(context, "there's no fine/extra-fine porta up/down!");
      goto noeffectmemory;
      break;

    case EFF_TONE_PORTA: goto noeffectmemory; break;

    case EFF_VIBRATO:
      if (!main_context.handle_effect_memory && !parameter) {
        print_warning_pattern(context, "there's no effect memory with this effect, this will be treated as a no-op.");
        break;
      }
      print_warning_pattern(context, "vibrato depth is doubled compared to other trackers!");
      break;

    case EFF_TREMOR:
      if (s3m_cwtv >= 0x1300 && s3m_cwtv < 0x1320)
        break;

    case EFF_ARPEGGIO: goto noeffectmemory; break;

    default: print_warning_pattern(context, "unsupported effect!"); break;
  }

  return effect;

noeffectmemory:
  if (!main_context.handle_effect_memory && !parameter)
    print_warning_pattern(context, "there's no effect memory with this effect, this will be treated as a no-op.");
  return effect;
}

void parse_s3m_pattern(FILE* file, usize position) {
  u8 channel = 0, row = 0, byte = 0;
  u8 note = 0xFF, ins = 0x00, volume = 0xFF, effect = 0x00, parameter = 0x00;

  if (!file || !position)
    return;
  if (feof(file) || ferror(file))
    return;

  fseek(file, (long)position, SEEK_SET);

  fseek(file, 2, SEEK_CUR);

  flush_s3m_pattern_array();

  while (row < MAXROWS) {
    byte = fgetb(file);

    if (!byte) {
      ++row;
      continue;
    }

    channel = (byte & 31);

    if (byte & 0x20)
      note = fgetb(file), ins = fgetb(file);
    else
      note = 0xFF, ins = 0x00;

    if (byte & 0x40)
      volume = fgetb(file);
    else
      volume = 0xFF;

    if (byte & 0x80)
      effect = fgetb(file), parameter = fgetb(file);
    else
      effect = 0x00, parameter = 0x00;

    s3m_unpacked_pattern[row][channel].note = note, s3m_unpacked_pattern[row][channel].ins = ins,
    s3m_unpacked_pattern[row][channel].vol = volume, s3m_unpacked_pattern[row][channel].eff = effect,
    s3m_unpacked_pattern[row][channel].prm = parameter;
  };

  if (main_context.verbose_mode)
    print_s3m_pattern();
}

u8 check_for_free_channel(usize row) {
  usize free_channel = 0;

  do {
    if (!s3m_unpacked_pattern[row][free_channel].eff)
      return (u8)free_channel;
  } while (++free_channel < S3M_MAXCHN);

  return 0xFF;
}

u8 search_for_last_nonzero_param(usize startingrow, usize c, usize effect) {
  usize i = startingrow;

  print_diagnostic("searching for last nonzero param for %c starting at row %02u and channel %02u", effect + EFFBASE, i,
                   c);

  while (i--) {
    /* print_diagnostic("checking row %02u", i); */
    if (!s3m_unpacked_pattern[i][c].prm || s3m_unpacked_pattern[i][c].eff != effect)
      continue;
    print_diagnostic("param is %02X", s3m_unpacked_pattern[i][c].prm);
    return s3m_unpacked_pattern[i][c].prm;
  }

  print_diagnostic("no matches found...");
  return 0;
}

u8 search_for_last_nonzero_param2(usize startingrow, usize channel, usize effect) {
  usize i = startingrow;
  const u8 effect_display = (u8)effect + EFFBASE;
  u8 lownib = 0, hinib = 0, param = 0;

  print_diagnostic("searching for last nonzero param for %c starting at row %02u and channel %02u", effect_display, i,
                   channel);

  i = startingrow;
  while (i--) {
    /* print_diagnostic("checking row %02u for low nibble", i); */

    if (!(s3m_unpacked_pattern[i][channel].prm & 0x0F) || s3m_unpacked_pattern[i][channel].eff != effect)
      continue;

    lownib = s3m_unpacked_pattern[i][channel].prm & 0x0F;
  }

  i = startingrow;
  while (i--) {
    /* print_diagnostic("checking row %02u for high nibble", i); */

    if (!(s3m_unpacked_pattern[i][channel].prm >> 4) || s3m_unpacked_pattern[i][channel].eff != effect)
      continue;

    hinib = s3m_unpacked_pattern[i][channel].prm >> 4;
  }

  if (!lownib || !hinib)
    goto nomatches;

  param = (hinib << 4) | lownib;
  print_diagnostic("param (low) is %1X and param (high) is %1X, forming %02X", lownib, hinib, param);
  return param;

nomatches:
  print_diagnostic("no matches found...");
  return 0;
}

void flush_s3m_pattern_array(void) {
  usize row = 0, channel = 0;
  do {
    for (channel = 0; channel < S3M_MAXCHN; ++channel) {
      s3m_unpacked_pattern[row][channel].note = 0xFF, s3m_unpacked_pattern[row][channel].ins = 0x00,
      s3m_unpacked_pattern[row][channel].vol = 0xFF, s3m_unpacked_pattern[row][channel].eff = 0x00,
      s3m_unpacked_pattern[row][channel].prm = 0x00;
    }
  } while (++row < MAXROWS);
}

u8 handle_effect_memory(Pattern_Context* context) {
  const u8 row = context->row, channel = context->channel;
  const u8 effect = context->effect, lastprm = search_for_last_nonzero_param(row, channel, effect);
  u8 parameter = context->parameter;

  if (!main_context.handle_effect_memory)
    return parameter;
  if (!row || parameter)
    return parameter;

  if (lastprm) {
    s3m_unpacked_pattern[row][channel].prm = lastprm;
    return lastprm;
  }

  return parameter;
}

u8 handle_effect_memory_separatenibs(Pattern_Context* context) {
  const usize row = context->row, channel = context->channel;
  const u8 effect = context->effect, parameter = context->parameter, hinib = parameter >> 4, lownib = parameter & 0x0F;
  const u8 lastprm = search_for_last_nonzero_param2(row, channel, effect);

  if (!main_context.handle_effect_memory)
    return parameter;
  if (!row || (lownib || hinib))
    return parameter;

  if (lastprm) {
    s3m_unpacked_pattern[row][channel].prm = lastprm;
    return lastprm;
  }

  return parameter;
}

void handle_s3m_effect(Pattern_Context* context) {
  const usize row = context->row;
  const bool st2vib_flag = GET_BIT(s3m_song_header.flags, S3M_ST2VIB);
  u8 freechn = check_for_free_channel(row);
  u8 effect = context->effect, parameter = context->parameter;
  u8 hinib = parameter >> 4, lownib = parameter & 0x0F, adjusted_vibrato_depth = 0;

  switch (check_effect(context)) {

    case EFF_SET_TEMPO:
      /* TODO: implement speed factor */
      parameter = lownib << 4;
      break;

    case EFF_SET_POSITION:
      if (freechn >= STM_MAXCHN)
        break;
      s3m_unpacked_pattern[row][freechn].eff = EFF_PATTERN_BREAK;
      break;

    case EFF_PATTERN_BREAK: parameter = 0; break;

    case EFF_VOLUME_SLIDE:
      parameter = handle_effect_memory_separatenibs(context);
      hinib = parameter >> 4, lownib = parameter & 0x0F;

      if (hinib == 0xF && lownib != 0) {
        hinib = 0;
        if (lownib >> 1)
          lownib >>= 1;
        else
          print_warning_pattern(context, "Failed to adjust fine volume slide.");
      } else if (lownib == 0xF && hinib != 0) {
        lownib = 0;
        if (hinib >> 1)
          hinib >>= 1;
        else
          print_warning_pattern(context, "Failed to adjust fine volume slide.");
      }

      parameter = (hinib << 4) | lownib;
      break;

    case EFF_PORTA_DOWN:
    case EFF_PORTA_UP:
      parameter = handle_effect_memory(context);
      hinib = parameter >> 4, lownib = parameter & 0x0F;

      if (hinib == 0xF) {
        hinib = 0;
        if (lownib >> 2)
          lownib >>= 2;
        else if (lownib >> 1)
          lownib >>= 1;
        else
          print_warning_pattern(context, "Failed to adjust fine portamento.");
      } else if (hinib == 0xE) {
        hinib = 0;
        if (lownib >> 3)
          lownib >>= 3;
        else if (lownib >> 2)
          lownib >>= 2;
        else if (lownib >> 1)
          lownib >>= 1;
        else
          print_warning_pattern(context, "Failed to adjust extra-fine portamento.");
      }

      parameter = (hinib << 4) | lownib;
      break;

    case EFF_TONE_PORTA: parameter = handle_effect_memory(context); break;

    case EFF_VIBRATO:
      parameter = handle_effect_memory_separatenibs(context);
      lownib = parameter & 0x0F;

      adjusted_vibrato_depth = lownib >> 1;

      if (adjusted_vibrato_depth) {
        if (!st2vib_flag) {
          print_diagnostic("adjusting vibrato depth from %u to %u.", lownib, adjusted_vibrato_depth);
          lownib = adjusted_vibrato_depth;
          print_diagnostic("adjustment successful!");
        }
      } else if (lownib) {
        print_diagnostic("adjustment failed... depth %u turned into %u. this will not be adjusted!", lownib,
                         adjusted_vibrato_depth);
      }
      break;

    case EFF_TREMOR:
      /* newer scream tracker 3 versions actually have memory for this effect.. */
      if (s3m_cwtv >= 0x1300 && s3m_cwtv < 0x1320)
        break;

      parameter = handle_effect_memory_separatenibs(context);
      break;

    case EFF_ARPEGGIO: parameter = handle_effect_memory_separatenibs(context); break;

    default: effect = 0, parameter = 0; break;
  };

  context->effect = effect, context->parameter = parameter;
}

void convert_s3m_pattern_to_stm(void) {
  usize row = 0, channel = 0;
  u8 proper_octave = 0;
  Pattern_Context pattern;

  blank_stm_pattern();

  do {
    for (channel = 0; channel < STM_MAXCHN; ++channel) {
      pattern.row = (u8)row, pattern.channel = (u8)channel;
      pattern.note = s3m_unpacked_pattern[row][channel].note,
      pattern.instrument = s3m_unpacked_pattern[row][channel].ins,
      pattern.volume = s3m_unpacked_pattern[row][channel].vol, pattern.effect = s3m_unpacked_pattern[row][channel].eff,
      pattern.parameter = s3m_unpacked_pattern[row][channel].prm;

      if (pattern.note < 0xFE)
        proper_octave = (pattern.note >> 4) - 2, pattern.note = (proper_octave << 4) | (pattern.note & 0x0F);

      if (pattern.volume > 64)
        pattern.volume = 65;

      handle_s3m_effect(&pattern);

      stm_pattern[row][channel][0] = pattern.note,
      stm_pattern[row][channel][1] = ((pattern.instrument & 31) << 3) | (pattern.volume & 7),
      stm_pattern[row][channel][2] = ((pattern.volume & 0x78) << 1) | (pattern.effect & 15),
      stm_pattern[row][channel][3] = pattern.parameter;
    }
  } while (++row < MAXROWS);
}

void blank_stm_pattern(void) {
  usize row = 0, channel = 0;
  do {
    for (channel = 0; channel < STM_MAXCHN; ++channel) {
      stm_pattern[row][channel][0] = 0xFF, stm_pattern[row][channel][1] = 0x01, stm_pattern[row][channel][2] = 0x80,
      stm_pattern[row][channel][3] = 0x00;
    }
  } while (++row < MAXROWS);
}

void convert_s3m_pattern_to_stx(FILE* file) {
  u8 row = 0, channel = 0, byte = 0;
  bool hasnote = false, hasvol = false, haseff = false;

  for (row = 0; row < MAXROWS; ++row) {
    for (channel = 0; channel < STX_MAXCHN; ++channel) {
      hasnote = (s3m_unpacked_pattern[row][channel].note != 0xFF) || (s3m_unpacked_pattern[row][channel].ins);
      hasvol = s3m_unpacked_pattern[row][channel].vol <= 64;
      haseff = s3m_unpacked_pattern[row][channel].eff || s3m_unpacked_pattern[row][channel].prm;

      if (!hasnote & !hasvol & !haseff)
        continue;

      byte = channel;

      if (hasnote)
        byte |= 0x20;
      if (hasvol)
        byte |= 0x40;
      if (haseff)
        byte |= 0x80;

      fputc(byte, file);

      if (hasnote) {
        fputc(s3m_unpacked_pattern[row][channel].note, file);
        fputc(s3m_unpacked_pattern[row][channel].ins, file);
      }
      if (hasvol)
        fputc(s3m_unpacked_pattern[row][channel].vol, file);
      if (haseff) {
        fputc(s3m_unpacked_pattern[row][channel].eff, file);
        fputc(s3m_unpacked_pattern[row][channel].prm, file);
      }
    }

    fputc(0, file);
  }
}
