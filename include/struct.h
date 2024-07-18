#ifndef __STRUCT_H
#define __STRUCT_H

#include "ext.h"

/* RM: stealing cs127's NTCheck's return values! */
enum FOC_ReturnCode {
  FOC_SUCCESS = 0x00,
  FOC_OPEN_FAILURE = 0x01,
  FOC_NOT_S3M_FILE = 0x02,
  FOC_MALFORMED_FILE = 0x04,
  FOC_CONV_FAILURE = 0x08,
  FOC_ALLOC_FAIL = 0x10,
  FOC_MALFORMED_BUFFER = 0x20,
  FOC_NO_FILENAMES = 0x40,
  FOC_SAMPLE_FAIL = 0x80
};

typedef struct {
  FILE* infile;
  FILE* outfile;

  struct {
    bool verbose_mode;
    bool sanitize_sample_names;
    bool handle_effect_memory;
  } flags;

  struct {
    u8 original_order_count;
    u8 order_count;
    u8 sample_count;
    u8 pattern_count;
  } stats;

} internal_state_t;

extern internal_state_t main_context;

#endif
