#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"
#include "s3m.h"
#include "stm.h"

#include "header.h"
#include "sample.h"

void show_s3m_song_header(void) {
  printf( "Song title: %s\n"
          "Global volume: %u\n"
          "Initial speed/tempo: %02X/%02X\n"
          "Song flags: %02X\n",
          s3m_song_header,
          (unsigned int)s3m_song_header[48],
          (unsigned int)s3m_song_header[49],
          (unsigned int)s3m_song_header[50],
          (unsigned int)s3m_song_header[38]
        );
}

/* s3m_song_header is expected to be filled beforehand */
void convert_song_header(void) {
  strncpy((char*)stm_song_header, (char*)s3m_song_header, 19);

  if (s3m_song_header[38] & S3M_AMIGAFREQLIMITS)
    eputs("WARNING: Ignoring Amiga frequency limit");

  if (s3m_song_header[51] & 128)
    eputs("WARNING: Do not expect the song to play in stereo.");

  if (s3m_song_header[38] & S3M_ST2TEMPO)
    stm_song_header[32] = s3m_song_header[49];
  else
    /* TODO: deal with speed factor */
    stm_song_header[32] = s3m_song_header[49] << 4;

  /* global volume */
  stm_song_header[34] = s3m_song_header[48];

  stm_song_header[33] = pattern_count;
}

void convert_song_orders(u8* s3m_order_array, usize length) {
  usize i = 0;
  for (; i < STM_ORDER_LIST_SIZE; i++) {
    stm_order_list[i] = (s3m_order_array[i] >= STM_MAXPAT)
                        ? s3m_order_array[i]
                        : STM_ORDER_END;
    if (i >= length) return;
  }
}

void convert_s3m_intstrument(usize id, bool isblank) {
  usize i = 0,
        type = s3m_inst_header[0], flags = s3m_inst_header[31];
  u16 length = s3m_inst_header[17] << 8 | s3m_inst_header[16],
      parapointer = convert_to_parapointer(id, length);
  

  switch(type) {
    case 0:
    /* instrument name */
    if (s3m_inst_header[1] != 0)
      memcpy((char *)stm_sample_header, (char *)&s3m_inst_header[1], 12);
    else if (s3m_inst_header[48] != 0) {
      strncpy((char *)stm_sample_header, (char *)&s3m_inst_header[48], 12);
    } else {
      memset(stm_sample_header, 0, 12);
    }

    /* instrument disk */
    stm_sample_header[13] = 0;

    /* lengths */
    stm_sample_header[17] = 0, stm_sample_header[16] = 0;

    /* loop points */
    stm_sample_header[19] = 0, stm_sample_header[18] = 0;
    stm_sample_header[21] = 0xFF, stm_sample_header[20] = 0xFF;

    /* volume */
    stm_sample_header[22] = 0;

    /* c2spd */
    stm_sample_header[25] = 0x21, stm_sample_header[24] = 0;
    break;

    case 1:
    /* instrument name */
    if (s3m_inst_header[1] != 0)
      memcpy((char *)stm_sample_header, (char *)&s3m_inst_header[1], 12);
    else if (s3m_inst_header[48] != 0) {
      strncpy((char *)stm_sample_header, (char *)&s3m_inst_header[48], 7);
      for(i = 0; i < 7; i++) {
        if(stm_sample_header[i] == ' ') {
          stm_sample_header[i] = '_';
        }
      }
      stm_sample_header[8] = '.',
      stm_sample_header[9] = (crc32(s3m_inst_header, 80) | '0') & '9',
      stm_sample_header[10] = (crc32(s3m_inst_header, 80) | '0') & '9',
      stm_sample_header[11] = (crc32(s3m_inst_header, 80) | '0') & '9';
    } else {
      for(i = 0; i < 7; i++) {
        stm_sample_header[i] = (crc32(s3m_inst_header, 80) | '0') & '9';
      }
      stm_sample_header[8] = '.',
      stm_sample_header[9] = (crc32(s3m_inst_header, 80) | '0') & '9',
      stm_sample_header[10] = (crc32(s3m_inst_header, 80) | '0') & '9',
      stm_sample_header[11] = (crc32(s3m_inst_header, 80) | '0') & '9';
    }

    /* instrument disk */
    stm_sample_header[13] = 0;

    /* lengths */
    stm_sample_header[17] = s3m_inst_header[17], stm_sample_header[16] = s3m_inst_header[16];
    
    /* loop points */
    if(flags & S3MSMP_LOOP) {
      stm_sample_header[19] = s3m_inst_header[21], stm_sample_header[18] = s3m_inst_header[20];
      stm_sample_header[21] = s3m_inst_header[25], stm_sample_header[20] = s3m_inst_header[24];
    } else {
      stm_sample_header[19] = 0, stm_sample_header[18] = 0;
      stm_sample_header[21] = 0xFF, stm_sample_header[20] = 0xFF;
    }
    
    /* volume */
    stm_sample_header[22] = s3m_inst_header[28];

    /* c2spd */
    stm_sample_header[25] = s3m_inst_header[33], stm_sample_header[24] = s3m_inst_header[32];

    /* parapointer */
    stm_sample_header[31] = parapointer >> 8, stm_sample_header[30] = parapointer & 0xFF;
    break;

    default:
      puts("WARNING: Adlib is not supported!");
    break;
  }
}
