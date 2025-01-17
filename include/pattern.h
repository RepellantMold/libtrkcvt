#ifndef __PATTERN_H
#define __PATTERN_H
#include "ext.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"
#include "fmt/stx.h"

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

void parse_s3m_pattern(FILE* file, usize position);

void convert_s3m_pattern_to_stm(stm_pattern_t* stm_pattern);
void convert_s3m_pattern_to_stx(FILE* file);

void flush_s3m_pattern_array(void);
void blank_stm_pattern(stm_pattern_t* stm_pattern);

void print_s3m_pattern(usize max);

void print_warning_pattern(Pattern_Context* context, const char* format, ...);

#endif
