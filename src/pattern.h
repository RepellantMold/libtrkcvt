#ifndef __PATTERN_H
#define __PATTERN_H
#include "ext.h"

#define MAXROWS 64

void parse_s3m_pattern(FILE* file, usize position);
void check_effect(u8 effect, u8 parameter, u8 row, u8 channel);
void convert_s3m_pattern_to_stm(void);

void print_s3m_pattern(void);

void warning_pattern_puts(u8 row, u8 channel, u8 effect, const char* msg);
void warning_pattern_printf(u8 row, u8 channel, u8 effect, const char* format, ...);

/* for display purposes */
u8 notetable[12][2] = {
  "C-", "C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"
};

#endif
