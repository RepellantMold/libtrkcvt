#ifndef __STX_H
#define __STX_H
#include <stdio.h>
#include <stdlib.h>

#include "../ext.h"

#define STX_ORDERPOS        1024
#define STX_MAXPAT          63
#define STX_MAXCHN          32
#define STX_MAXSMP          96

#define STX_ORDERMULTIPLIER 5

extern u8 stx_song_header[64];

typedef struct {
  // little endian.
  u8 upper;
  u8 lower1;
  u8 lower2;
} stx_pcm_parapointers;

typedef struct {
  u16 patpara_table_pos;
  u16 inspara_table_pos;
  u16 chn_table_pos;
} stx_parapointers;

extern u16 stx_inst_pointers[STX_MAXSMP];
extern u16 stx_pat_pointers[STX_MAXPAT];
extern stx_pcm_parapointers stx_pcm_pointers[STX_MAXSMP];

#endif
