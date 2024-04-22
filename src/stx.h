#ifndef __STX_H
#define __STX_H
#include <stdio.h>
#include <stdlib.h>
#include "ext.h"

#define STX_ORDERPOS        1024
#define STX_MAXPAT          63
#define STX_MAXCHN          32
#define STX_MAXSMP          96

#define STX_ORDERMULTIPLIER 5

/* NOTE : the STX instrument header format is identical to S3M's (minus 'SRCS'), so it's not included here.
I'm also mentioning format differences for completeness. */

u8 stx_song_header[64] = {
    // song title (ASCIIZ)
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0',

    // tracker name
    '!', 'S', 'c', 'r', 'e', 'a', 'm', '!',

    // DOS EOF (1.1)/pattern size (1.0)
    0x1A, 0,

    // reserved
    0, 0,

    // pattern table offset
    0, 0,

    // sample table offset
    0, 0,

    // channel table offset
    0, 0,

    // reserved
    0, 0, 0, 0,

    // global volume
    64,

    // initial tempo
    0x60,

    // reserved
    1, 0, 0, 0,

    // number of patterns
    0, 0,

    // number of samples
    0, 0,

    // number of orders
    0, 0,

    // unknown
    0, 0, 0, 0, 0, 0,

    // magic
    'S', 'C', 'R', 'M'};

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

u16 stx_inst_pointers[STX_MAXSMP] = {0};
u16 stx_pat_pointers[STX_MAXPAT] = {0};
stx_pcm_parapointers stx_pcm_pointers[STX_MAXSMP] = {0};

#endif
