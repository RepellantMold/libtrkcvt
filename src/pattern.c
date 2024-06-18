#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "envcheck.h"
#include "ext.h"

#include "file.h"
#include "log.h"
#include "main.h"
#include "pattern.h"
#include "effects.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"

s3mevent_t unpacked_pattern[64][32] = {{{0xFF, 0x00, 0xFF, 0x00, 0x00}}};

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

static void format_s3m_row(char* buffer, usize row, usize max) {
  register usize channel = 0;
  u8 note, ins, volume, effect, parameter;
  register int offset = 0;

  do {
    note = unpacked_pattern[row][channel].note;
    ins = unpacked_pattern[row][channel].ins;
    volume = unpacked_pattern[row][channel].vol;
    effect = unpacked_pattern[row][channel].eff;
    parameter = unpacked_pattern[row][channel].prm;

    if (note < 0xFE)
      offset += sprintf(buffer + offset, "%.2s%01u", notetable[note & 0x0F], (note >> 4) + 1);
    else if (note == 0xFE)
      offset += sprintf(buffer + offset, "^^^");
    else if (note == 0xFF)
      offset += sprintf(buffer + offset, "...");

    if (ins)
      offset += sprintf(buffer + offset, " %02u ", ins);
    else
      offset += sprintf(buffer + offset, " .. ");

    if (volume <= 64)
      offset += sprintf(buffer + offset, "%02u ", volume);
    else
      offset += sprintf(buffer + offset, ".. ");

    if (effect)
      offset += sprintf(buffer + offset, "%c%02X ", EFFBASE + effect, parameter);
    else
      offset += sprintf(buffer + offset, "... ");
  } while (++channel < max);
}

void print_s3m_pattern(usize max_channels_shown) {
  register usize row = 0;
  char* buffer;

  buffer = (char*)calloc(14, MAXROWS * max_channels_shown);
  if (!buffer) {
    print_error("failed to allocate buffer!");
    exit(1);
  }

  for (; row < MAXROWS; ++row) {
    format_s3m_row(buffer, row, max_channels_shown);
    print_diagnostic("r:%02u = %s", row, buffer);
  }

  free(buffer);
}

void parse_s3m_pattern(FILE* file, usize position) {
  register u8 channel = 0, row = 0, byte = 0;
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

    unpacked_pattern[row][channel].note = note, unpacked_pattern[row][channel].ins = ins,
    unpacked_pattern[row][channel].vol = volume, unpacked_pattern[row][channel].eff = effect,
    unpacked_pattern[row][channel].prm = parameter;
  };

  if (main_context.flags.verbose_mode)
    print_s3m_pattern(STM_MAXCHN);
}

void flush_s3m_pattern_array(void) {
  register usize row = 0, channel = 0;
  do {
    for (channel = 0; channel < S3M_MAXCHN; ++channel) {
      unpacked_pattern[row][channel].note = 0xFF, unpacked_pattern[row][channel].ins = 0x00,
      unpacked_pattern[row][channel].vol = 0xFF, unpacked_pattern[row][channel].eff = 0x00,
      unpacked_pattern[row][channel].prm = 0x00;
    }
  } while (++row < MAXROWS);
}

void convert_s3m_pattern_to_stm(void) {
  usize row = 0, channel = 0;
  u8 proper_octave = 0;
  Pattern_Context pattern;

  blank_stm_pattern();

  do {
    for (channel = 0; channel < STM_MAXCHN; ++channel) {
      pattern.row = (u8)row, pattern.channel = (u8)channel;
      pattern.note = unpacked_pattern[row][channel].note,
      pattern.instrument = unpacked_pattern[row][channel].ins,
      pattern.volume = unpacked_pattern[row][channel].vol, pattern.effect = unpacked_pattern[row][channel].eff,
      pattern.parameter = unpacked_pattern[row][channel].prm;

      if (pattern.note < 0xFE) {
        proper_octave = (u8)((pattern.note >> 4) - 2),
        pattern.note = (u8)((proper_octave << 4) | (pattern.note & 0x0F));
      }

      if (pattern.volume > 64)
        pattern.volume = 65;

      handle_s3m_effect(&pattern);

      stm_pattern[row][channel][0] = pattern.note,
      stm_pattern[row][channel][1] = (u8)((pattern.instrument & 31) << 3) | (pattern.volume & 7),
      stm_pattern[row][channel][2] = (u8)((pattern.volume & 0x78) << 1) | (pattern.effect & 15),
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
      hasnote = (unpacked_pattern[row][channel].note != 0xFF) || (unpacked_pattern[row][channel].ins);
      hasvol = unpacked_pattern[row][channel].vol <= 64;
      haseff = unpacked_pattern[row][channel].eff || unpacked_pattern[row][channel].prm;

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
        fputc(unpacked_pattern[row][channel].note, file);
        fputc(unpacked_pattern[row][channel].ins, file);
      }
      if (hasvol)
        fputc(unpacked_pattern[row][channel].vol, file);
      if (haseff) {
        fputc(unpacked_pattern[row][channel].eff, file);
        fputc(unpacked_pattern[row][channel].prm, file);
      }
    }

    fputc(0, file);
  }
}
