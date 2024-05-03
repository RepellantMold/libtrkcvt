#ifndef __PATTERN_H
#define __PATTERN_H
#include "ext.h"

#define EFFBASE ('A' - 1)
#define EFF(e)  (e - EFFBASE)

#define MAXROWS 64

typedef struct {
  u8 row;
  u8 channel;
  u8 note;
  u8 instrument;
  u8 volume;
  u8 effect;
  u8 parameter;
} Pattern_Context;

// STM and STX use the same set of effects.
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
  EFF_ARPEGGIO = EFF('J'),
};

void parse_s3m_pattern(FILE* file, usize position);

int check_effect(Pattern_Context* context);

void convert_s3m_pattern_to_stm(void);
void convert_s3m_pattern_to_stx(FILE* file);

void flush_s3m_pattern_array(void);
void blank_stm_pattern(void);

void print_s3m_row(usize r);
void print_s3m_pattern(void);

void warning_pattern_puts(Pattern_Context* context, const char* msg);
void warning_pattern_printf(Pattern_Context* context, const char* format, ...);

#endif
