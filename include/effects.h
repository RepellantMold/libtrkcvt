#ifndef __EFFECTS_H
#define __EFFECTS_H
#include "ext.h"

#include "pattern.h"

#include "fmt/s3m.h"

extern s3mevent_t unpacked_pattern[64][32];

#define EFFBASE ('A' - 1)
#define EFF(e)  (e - EFFBASE)

/* STM and STX use the same set of effects. */
enum Effects {
  EFF_NO_EFFECT = 0,
  EFF_SET_TEMPO = EFF('A'),
  EFF_SET_POSITION = EFF('B'),
  EFF_PATTERN_BREAK = EFF('C'),
  EFF_VOLUME_SLIDE = EFF('D'),
  EFF_PORTA_DOWN = EFF('E'),
  EFF_PORTA_UP = EFF('F'),
  EFF_TONE_PORTA = EFF('G'),
  EFF_VIBRATO = EFF('H'),
  EFF_TREMOR = EFF('I'),
  EFF_ARPEGGIO = EFF('J')
};

int check_effect(Pattern_Context* context);

u8 search_for_last_nonzero_param(usize startingrow, usize c, usize effect);
u8 search_for_last_nonzero_param2(usize startingrow, usize channel, usize effect);
u8 handle_effect_memory(Pattern_Context* context);
u8 handle_effect_memory_separatenibs(Pattern_Context* context);
void handle_s3m_effect(Pattern_Context* context);
int check_effect(Pattern_Context* context);
u8 check_for_free_channel(usize row);

#endif
