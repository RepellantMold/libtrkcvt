#ifndef __PATTERN_H
#define __PATTERN_H
#include "ext.h"

#define MAXROWS 64

typedef struct {
  u8 s3m_unpacked_pattern[64][32][5];
  u8 stm_pattern[64][4][4];
  u8* stx_pattern;
} Pattern_Context;

typedef struct {
  u8 row;
  u8 channel;
  u8 effect;
  u8 parameter;
} Pattern_Display_Context;

void parse_s3m_pattern(FILE* file, usize position);
int check_effect(Pattern_Display_Context* context);
void convert_s3m_pattern_to_stm(void);

void flush_s3m_pattern_array(void);
void blank_stm_pattern(void);

void print_s3m_pattern(void);

void warning_pattern_puts(Pattern_Display_Context* context, const char* msg);
void warning_pattern_printf(Pattern_Display_Context* context, const char* format, ...);

/* for display purposes */
u8 notetable[12][2] = {"C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"};

#endif
