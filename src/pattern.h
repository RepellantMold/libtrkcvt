#ifndef __PATTERN_H
#define __PATTERN_H
#include "ext.h"

#define MAXROWS 64

void parse_s3m_pattern(FILE* file, usize position);
void check_effect(u8 effect, u8 parameter);
void parse_s3m_pattern(FILE* file, usize position);
void convert_s3m_pattern_to_stm(void);

/* for display purposes */
u8 notetable[12][2] = {
  "C-", "C#","D-","D#","E-","F-","F#","G-","G#","A-","A#","B-"
};

#endif
