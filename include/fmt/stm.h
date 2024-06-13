#ifndef __STM_H
#define __STM_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ext.h"

#define STM_MAXCHN          4
#define STM_MAXSMP          31

/* Strangely, in ST2, you can actually add orders above 63 (it even writes the bytes to the file),
but the pattern editor does not go above 63 and no players will accept this so... */
#define STM_MAXPAT          63

#define STM_ORDER_END       99
#define STM_ORDER_LIST_SIZE 128

#define STM_PATSIZE         ((4 * 4) * 64)

typedef struct {
  char filename[12];
  char zero;
  u8 disk;
  u16 parapointer;
  u16 length;
  u16 loop_start;
  u16 loop_end;
  u8 default_volume;
  char reserved1;
  u16 c_speed;
  char reserved2[6];
} stm_instrument_header_t;

typedef struct {
  char title[20];
  char tracker[8];
  char dos_eof;
  char type;

  union {
    struct {
      u8 major;
      u8 minor;
    } bytes;

    u16 word;
  } version;

  u8 initial_tempo;

  u8 total_patterns;
  u8 global_volume;
  char reserved1[13];

  stm_instrument_header_t instruments[31];

} stm_song_header_t;

extern stm_song_header_t stm_song_header;

extern u8 stm_sample_data[USHRT_MAX];

extern u8 stm_order_list[STM_ORDER_LIST_SIZE];

extern u8 stm_pattern[64][4][4];

extern u16 stm_pcm_pointers[STM_MAXSMP];

#endif
