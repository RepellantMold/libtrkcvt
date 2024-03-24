#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

void show_s3m_header(void) {
  printf("Song title: %s", s3m_song_header);
}

/* S3M is expected to be at the start,
 * and for s3m_song_header to be filled */
void convert_song_header(void) {
  strncpy((char *)stm_song_header, (char *)s3m_song_header, 20);

  if (s3m_song_header[38] & S3M_ENABLEFILTER)
    puts("WARNING: Ignoring Amiga frequency limit");

  /* TODO: deal with speed factor */
  stm_song_header[20] = s3m_song_header[49] << 4;

  stm_song_header[22] = s3m_song_header[48];
}

void convert_song_orders(unsigned char *s3m_order_array, usize length) {
  usize i = 0;
  for (; i < STM_ORDER_LIST_SIZE; i++) {
    stm_order_list[i] = (s3m_order_array[i] >= STM_MAXPAT)
                        ? s3m_order_array[i]
                        : STM_ORDER_END;
    if (i >= length) break;
  }
}
