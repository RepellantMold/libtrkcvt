#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "envcheck.h"
#include "ext.h"
#include "log.h"

#include "effects.h"
#include "main.h"
#include "pattern.h"


#include "fmt/s3m.h"
#include "fmt/stm.h"

u8 search_for_last_nonzero_param(usize startingrow, usize c, usize effect) {
  register usize i = startingrow;

  print_diagnostic("searching for last nonzero param for %c starting at row %02u and channel %02u", effect + EFFBASE, i,
                   c);

  while (i--) {
    /* print_diagnostic("checking row %02u", i); */
    if (!unpacked_pattern[i][c].prm || unpacked_pattern[i][c].eff != effect)
      continue;
    print_diagnostic("param is %02X", unpacked_pattern[i][c].prm);
    return unpacked_pattern[i][c].prm;
  }

  print_diagnostic("no matches found...");
  return 0;
}

u8 search_for_last_nonzero_param2(usize startingrow, usize channel, usize effect) {
  register usize i = startingrow;
  const u8 effect_display = (u8)effect + EFFBASE;
  register u8 lownib = 0, hinib = 0, param = 0;

  print_diagnostic("searching for last nonzero param for %c starting at row %02u and channel %02u", effect_display, i,
                   channel);

  i = startingrow;
  while (i--) {
    /* print_diagnostic("checking row %02u for low nibble", i); */

    if (!(unpacked_pattern[i][channel].prm & 0x0F) || unpacked_pattern[i][channel].eff != effect)
      continue;

    lownib = unpacked_pattern[i][channel].prm & 0x0F;
  }

  i = startingrow;
  while (i--) {
    /* print_diagnostic("checking row %02u for high nibble", i); */

    if (!(unpacked_pattern[i][channel].prm >> 4) || unpacked_pattern[i][channel].eff != effect)
      continue;

    hinib = unpacked_pattern[i][channel].prm >> 4;
  }

  if (!lownib || !hinib)
    goto nomatches;

  param = (u8)((hinib << 4) | lownib);
  print_diagnostic("param (low) is %1X and param (high) is %1X, forming %02X", lownib, hinib, param);
  return param;

nomatches:
  print_diagnostic("no matches found...");
  return 0;
}

u8 handle_effect_memory(Pattern_Context* context) {
  const u8 row = context->row, channel = context->channel;
  const u8 effect = context->effect, lastprm = search_for_last_nonzero_param(row, channel, effect),
           parameter = context->parameter;

  if (!main_context.flags.handle_effect_memory)
    return parameter;
  if (!row || parameter)
    return parameter;

  if (lastprm) {
    unpacked_pattern[row][channel].prm = lastprm;
    return lastprm;
  }

  return parameter;
}

u8 handle_effect_memory_separatenibs(Pattern_Context* context) {
  const usize row = context->row, channel = context->channel;
  const u8 effect = context->effect, parameter = context->parameter, hinib = parameter >> 4, lownib = parameter & 0x0F,
           lastprm = search_for_last_nonzero_param2(row, channel, effect);

  if (!main_context.flags.handle_effect_memory)
    return parameter;
  if (!row || (lownib || hinib))
    return parameter;

  if (lastprm) {
    unpacked_pattern[row][channel].prm = lastprm;
    return lastprm;
  }

  return parameter;
}

void handle_s3m_effect(Pattern_Context* context) {
  const usize row = context->row;
  const bool st2vib_flag = GET_BIT(s3m_song_header.flags, S3M_ST2VIB);
  register u8 freechn = check_for_free_channel(row), effect = context->effect, parameter = context->parameter,
              hinib = parameter >> 4, lownib = parameter & 0x0F, adjusted_vibrato_depth = 0;

  switch (check_effect(context)) {

    case EFF_SET_TEMPO:
      /* TODO: implement speed factor */
      parameter = (u8)(lownib << 4);
      break;

    case EFF_SET_POSITION:
      if (freechn >= STM_MAXCHN)
        break;
      unpacked_pattern[row][freechn].eff = EFF_PATTERN_BREAK;
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

      parameter = (u8)((hinib << 4) | lownib);
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

      parameter = (u8)((hinib << 4) | lownib);
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
      if (!main_context.flags.handle_effect_memory && !parameter) {
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
  if (!main_context.flags.handle_effect_memory && !parameter)
    print_warning_pattern(context, "there's no effect memory with this effect, this will be treated as a no-op.");
  return effect;
}

u8 check_for_free_channel(usize row) {
  register usize free_channel = 0;

  do {
    if (!unpacked_pattern[row][free_channel].eff)
      return (u8)free_channel;
  } while (++free_channel < S3M_MAXCHN);

  return 0xFF;
}
