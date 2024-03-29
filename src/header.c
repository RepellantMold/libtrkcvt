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
#include "parapnt.h"

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

void show_s3m_inst_header(void) {
  printf( "Sample name/filename: %.28s/%.12s\n"
          "Default volume: %02u\n"
          "Sample flags: %01X\n"
          "C frequency: %06u\n"
          "Length/Loop start/end: %06u/%06u/%06u\n",
          &s3m_inst_header[48],
          &s3m_inst_header[1],
          (unsigned int)s3m_inst_header[28],
          (unsigned int)s3m_inst_header[31],
          (unsigned int)(s3m_inst_header[33] << 8 | s3m_inst_header[32]),
          (unsigned int)(s3m_inst_header[17] << 8 | s3m_inst_header[16]),
          (unsigned int)(s3m_inst_header[21] << 8 | s3m_inst_header[20]),
          (unsigned int)(s3m_inst_header[25] << 8 | s3m_inst_header[24])
        );
}

void grab_s3m_parapointers(FILE* file) {
  usize i = 0;
  usize count = 0;

  if (!file || feof(file) || ferror(file)) return;

  fseek(file, S3M_ORDERPOS, SEEK_SET);

  (void)!fread(s3m_order_array, sizeof(u8), order_count, file);

  /* see section "2.6 Load Order Data" from "FireLight S3M Player Tutorial.txt" */
  for(count = 0; count < order_count; count++) {
    printf("Order %u -> %u\n", (int)count, (int)i);
    if (s3m_order_array[count] < S3M_ORDER_MARKER) {
      s3m_order_array[i] = s3m_order_array[count];
      i++;
      if(s3m_order_array[count] > pattern_count) 
        pattern_count = s3m_order_array[count];
    }
  }
  order_count = i;

  (void)!fread(s3m_inst_pointers, sizeof(u16), sample_count, file);

  for(i = 0; i < sample_count; i++) {
    s3m_inst_pointers[i] = (u16)convert_from_parapointer(s3m_inst_pointers[i]);
  }

  (void)!fread(s3m_pat_pointers, sizeof(u16), pattern_count, file);

  for(i = 0; i < pattern_count; i++) {
    s3m_pat_pointers[i] = (u16)convert_from_parapointer(s3m_pat_pointers[i]);
  }
}

void check_s3m_channels(void) {
  usize i = 0;
  usize channel = 0;

  while(i++ < S3M_MAXCHN) {
    channel = s3m_song_header[S3M_CHANNELARRAYPOS+(i-1)];

    if (channel == S3MCHN_DISABLED) break;

    if (i > STM_MAXCHN) {
      eputs("WARNING: There's more than 4 channels, they'll be truncated.");
      break;
    }

    if (channel & S3MCHN_MUTE)
      eprintf("WARNING: Channel %u is muted which is unsupported, the notes will be converted anyway.", i);

    if (channel >= S3MCHN_ADLIBMEL1 && channel <= S3MCHN_ADLIBHATDRUM)
      eprintf("WARNING: Adlib channel detected (channel %u), the notes will be converted as is without the instrument data.\n", i);
  }
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

void convert_song_orders(usize length) {
  usize i = 0;

  memset(stm_order_list, STM_ORDER_END, STM_ORDER_LIST_SIZE);

  do {
    if (i >= length) break;
    stm_order_list[i] = (s3m_order_array[i] >= STM_MAXPAT)
                        ? STM_ORDER_END
                        : s3m_order_array[i];
  } while (i++ < STM_ORDER_LIST_SIZE);
}

void grab_sample_data(FILE* file, usize position) {
  if (!file || feof(file) || ferror(file)) return;
  fseek(file, position, SEEK_SET);
  (void)!fread(s3m_inst_header, sizeof(u8), 80, file);
}

void convert_s3m_intstrument(void) {
  usize i = 0;
  const usize type = s3m_inst_header[0], flags = s3m_inst_header[31];
  u32 crc = crc32(s3m_inst_header, 80);

  switch(type) {
    case S3MSMPTYPE_MSG:
    generateblanksample:
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

    case S3MSMPTYPE_SMP:
    /* instrument name */
    if (s3m_inst_header[1] != 0)
      memcpy((char *)stm_sample_header, (char *)&s3m_inst_header[1], 12);
    else if (s3m_inst_header[48] != 0) {
      memcpy((char *)stm_sample_header, (char *)&s3m_inst_header[48], 8);
      for(i = 0; i < 8; i++) {
        /* sanitization for 8.3 filenames */
        if((stm_sample_header[i] <= ' ') || (stm_sample_header[i] >= 0x7F)) {
          stm_sample_header[i] = '_';
        }
      }
      stm_sample_header[8] = '.',
      snprintf((char*)&stm_sample_header[9], 4, "%lu", (u32)crc % 512);
    } else {
      snprintf((char*)&stm_sample_header[0], 8, "%lu", (u32)crc * rand());
      stm_sample_header[7] = '_',
      stm_sample_header[8] = '.',
      snprintf((char*)&stm_sample_header[9], 4, "%lu", (u32)crc % 512);
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
    break;

    default:
    eputs("WARNING: Adlib instrument is not supported, only converting sample name.");
    goto generateblanksample;
    break;
  }
}

u32 grab_s3m_pcm_pointer(void) {
  u32 parapointer = s3m_inst_header[13] << 16 | s3m_inst_header[15] << 8 | s3m_inst_header[14];
  parapointer <<= 4;
  printf("PCM Parapointer: %lX\n", parapointer);
  return parapointer;
}

u16 grab_s3m_pcm_len(void) {
  u16 length = s3m_inst_header[17] << 8 | s3m_inst_header[16];

  if((s3m_inst_header[19] << 8 | s3m_inst_header[18]) != 0)
    eprintf("WARNING: the sample is too long, only converting the first 64kb of it.\n");

  return length;
}

void generate_blank_stm_instrument(void) {
  memset(stm_sample_header, 0, 12);

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
}

