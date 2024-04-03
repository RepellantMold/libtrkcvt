#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"
#include "sample.h"
#include "parapnt.h"

u16 calculate_stm_sample_parapointer(void) {
  usize pos = (usize)ftell(main_context.outfile);
  return (u16)convert_to_parapointer(pos);
}

usize convert_to_parapointer(usize pointer) {
  optional_printf("%04X -> %04X\n", pointer, pointer >> 4);
  return pointer >> 4;
}

usize convert_from_parapointer(usize parapointer) {
  optional_printf("%04X -> %04X\n", parapointer, parapointer << 4);
  return parapointer << 4;
}
