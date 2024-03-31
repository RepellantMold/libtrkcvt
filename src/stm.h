#ifndef __STM_H
#define __STM_H
#include <stdlib.h>
#include <stdio.h>
#include "ext.h"

#define STM_MAXCHN 4
#define STM_MAXSMP 31
/*
 * Strangely, in ST2, you can actually add orders above 63
 * (it even writes the bytes to the file),
 * but the pattern editor does not go above 63 so...
 */
#define STM_MAXPAT 63
#define STM_ORDER_END 99
#define STM_ORDER_LIST_SIZE 128

#define STM_PATSIZE ((4*4)*64)

u8 stm_song_header[48] = {
  /* song title (ASCIIZ) */
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

  /* tracker name */
  '!', 'S', 'c', 'r', 'e', 'a', 'm', '!',

  0x1A,

  /* file type */
  2,

  /* version */
  0x02,
  0x15,

  /* tempo */
  0x60,

  /* number of patterns */
  0,

  /* global volume */
  0x40,

  /* reserved (???) */
  0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58
};

u8 stm_sample_header[32] = {
  /* filename (ASCIIZ) */
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

  /* zero */
  0,

  /* instrument disk */
  0,

  /* reserved */
  0, 0,

  /* length in bytes */
  0, 0,

  /* loop start */
  0, 0,

  /* loop end */
  0xFF, 0xFF,

  /* volume */
  64,

  /* reserved */
  0,

  /* speed for mid-C (in Hz) */
  0x00, 0x21,

  /* reserved */
  0, 0, 0, 0,

  /* internal segment address/(in modules:)length in paragraphs */
  0, 0
};

u8 stm_order_list[STM_ORDER_LIST_SIZE] = {STM_ORDER_END};

u8 stm_pattern[64][4][4] = {{{0xFF, 0x01, 0x80, 0x00}}};

#endif
