#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "header.h"

void show_s3m_song_header(void) {
  printf( "Song title: %s\n"
          "Global volume: %d\n"
          "Initial speed/tempo: %02X/%02X\n"
          "Song flags: %02X\n",
          s3m_song_header,
          s3m_song_header[48],
          s3m_song_header[49],
          s3m_song_header[50],
          s3m_song_header[38]
        );
}

/* s3m_song_header is expected to be filled beforehand */
void convert_song_header(void) {
  (void)strncpy((char *)stm_song_header, (char *)s3m_song_header, 19);

  if (s3m_song_header[38] & S3M_ENABLEFILTER)
    puts("WARNING: Ignoring Amiga frequency limit");

  if (s3m_song_header[51] & 128)
    puts("WARNING: Do not expect the song to play in stereo.");

  /* TODO: deal with speed factor */
  stm_song_header[34] = s3m_song_header[49] << 4;

  /* global volume */
  stm_song_header[36] = s3m_song_header[48];

  stm_song_header[33] = pattern_count;
}

void convert_song_orders(unsigned char *s3m_order_array, usize length) {
  usize i = 0;
  for (; i < STM_ORDER_LIST_SIZE; i++) {
    stm_order_list[i] = (s3m_order_array[i] >= STM_MAXPAT)
                        ? s3m_order_array[i]
                        : STM_ORDER_END;
    if (i >= length) return;
  }
}
