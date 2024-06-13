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

typedef struct {
  char title[20];
  char tracker[8];

  union {
    u16 first_pattern_size;
    u8 dos_eof;
  } misc;

  u16 reserved1;
  u16 pattern_table_parapointer;
  u16 instrument_table_parapointer;
  u16 channel_table_parapointer;
  u32 reserved2;
  u8 global_volume;
  u8 initial_tempo;
  u8 reserved3;
  u16 total_patterns;
  u16 total_instruments;
  u16 total_orders;
  char reserved4[6];
  char scrm[4];
} stx_song_header_t;

extern stx_song_header_t stx_song_header;

typedef struct {
  // little endian.
  union {
    struct {
      u8 high;
      u8 low1;
      u8 low2;
    } bytes;

    struct {
      u8 high;
      u16 low;
    } words;

    u32 full;
  } memseg;
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
