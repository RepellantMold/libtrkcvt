#include "envcheck.h"
#include "ext.h"
#include "stm.h"

u8 stm_song_header[48] = {
  /* song title (ASCIIZ) */
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

  /* tracker name */
  '!', 'S', 'r', 'e', 'a', 'm', '!',

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

  /* reserved */
  0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58
};

u8 stm_sample_header[32] = {
  /* filename (ASCIIZ) */
  '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', 

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

void convert_song_orders(unsigned char *s3m_order_array, usize length) {
  usize i = 0;
  for (; i < STM_ORDER_LIST_SIZE; i++) {
    stm_song_header[i] = (s3m_order_array[i] >= STM_MAXPAT)
                         ? s3m_order_array
                         : STM_ORDER_END;
    if (i >= length) break;
  }
}
