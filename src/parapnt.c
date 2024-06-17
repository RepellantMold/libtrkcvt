#include <stdio.h>
#include <stdlib.h>

#include "envcheck.h"
#include "ext.h"

#include "file.h"
#include "log.h"
#include "main.h"
#include "parapnt.h"
#include "sample.h"
#include "song_header.h"


#include "fmt/s3m.h"
#include "fmt/stm.h"
#include "fmt/stx.h"

u16 calculate_stm_sample_parapointer(void) {
  const usize pos = (usize)ftell(main_context.outfile);
  return (u16)convert_to_parapointer(pos);
}

stx_pcm_parapointers calculate_stx_sample_parapointer(void) {
  const long pos = ftell(main_context.outfile);
  stx_pcm_parapointers parapointer;

  parapointer.memseg.bytes.high = (u8)(pos >> 16);
  parapointer.memseg.bytes.low1 = (u8)(pos >> 8);
  parapointer.memseg.bytes.low2 = (u8)pos;

  parapointer.memseg.full >>= 4;
  return parapointer;
}

usize convert_to_parapointer(usize pointer) {
  print_diagnostic("%04X -> %04X", pointer, pointer >> 4);
  return pointer >> 4;
}

usize convert_from_parapointer(usize parapointer) {
  print_diagnostic("%04X -> %04X", parapointer, parapointer << 4);
  return parapointer << 4;
}

void grab_s3m_parapointers(FILE* file) {
  register usize i = 0;

  if (!file || feof(file) || ferror(file))
    return;

  fseek(file, S3M_ORDERPOS + original_order_count, SEEK_SET);

  for (i = 0; i < sample_count; i++) {
    s3m_inst_pointers[i] = fgetw(file);
    print_diagnostic("Sample %zu:", i);
    s3m_inst_pointers[i] = (u16)convert_from_parapointer(s3m_inst_pointers[i]);
  }

  for (i = 0; i < pattern_count; i++) {
    s3m_pat_pointers[i] = fgetw(file);
    print_diagnostic("Pattern %zu:", i);
    s3m_pat_pointers[i] = (u16)convert_from_parapointer(s3m_pat_pointers[i]);
  }
}
