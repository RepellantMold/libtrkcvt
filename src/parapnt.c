#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"
#include "sample.h"
#include "parapnt.h"

/* TODO: if a sample is blank then it should be discarded
 * from the calculation & the next non-blank one should be next up */
u16 calculate_stm_sample_parapointer(usize id, usize sample_size) {
  usize pos = 1168 + (STM_PATSIZE * pattern_count) * id;
  pos += sample_size + calculate_sample_padding(sample_size);
  return pos >> 4;
}

usize convert_from_parapointer(usize parapointer) {
  return parapointer << 4;
}