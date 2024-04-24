#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"

#include "s3m.h"
#include "stm.h"
#include "stx.h"

#include "header.h"
#include "parapnt.h"
#include "sample.h"

#include "crc.c"

void show_s3m_song_header(void) {
  const u8 global_volume = s3m_song_header[48], initial_speed = s3m_song_header[49],
           initial_tempo = s3m_song_header[50], song_flags = s3m_song_header[38];

  printf("Song title: %s\n"
         "Global volume: %u\n"
         "Initial speed/tempo: %02X/%02X\n"
         "Song flags: %02X\n",
         s3m_song_header, global_volume, initial_speed, initial_tempo, song_flags);
}

void show_s3m_inst_header(void) {
  const u8 default_volume = s3m_inst_header[29], sample_flags = s3m_inst_header[31];
  const u16 c_frequency = s3m_inst_header[33] << 8 | s3m_inst_header[32],
            length = s3m_inst_header[17] << 8 | s3m_inst_header[16],
            loop_start = s3m_inst_header[21] << 8 | s3m_inst_header[20],
            loop_end = s3m_inst_header[25] << 8 | s3m_inst_header[24];

  printf("Sample name/filename: %.28s/%.12s\n"
         "Default volume: %02u\n"
         "Sample flags: %01X\n"
         "C frequency: %06u\n"
         "Length/Loop start/end: %06u/%06u/%06u\n",
         &s3m_inst_header[48], &s3m_inst_header[1], default_volume, sample_flags, c_frequency, length, loop_start,
         loop_end);
}

void grab_s3m_orders(FILE* file) {
  usize i = 0, count = 0;

  if (!file || feof(file) || ferror(file))
    return;

  fseek(file, S3M_ORDERPOS, SEEK_SET);

  (void)!fread(s3m_order_array, sizeof(u8), original_order_count, file);

  // see section "2.6 Load Order Data" from "FireLight S3M Player Tutorial.txt"
  for (count = 0; count < original_order_count; ++count) {
    if (s3m_order_array[count] < S3M_ORDER_MARKER) {
      s3m_order_array[i] = s3m_order_array[count];
      if (s3m_order_array[count] > pattern_count)
        pattern_count = s3m_order_array[count];
      optional_printf("Order %zu -> %zu\n", count, i++);
    }
  }
  order_count = (u8)i;
}

void grab_s3m_parapointers(FILE* file) {
  usize i = 0;

  if (!file || feof(file) || ferror(file))
    return;

  fseek(file, S3M_ORDERPOS + original_order_count, SEEK_SET);

  for (i = 0; i < sample_count; i++) {
    s3m_inst_pointers[i] = fgetw(file);
    optional_printf("Sample %zu:\n", i);
    s3m_inst_pointers[i] = (u16)convert_from_parapointer(s3m_inst_pointers[i]);
  }

  for (i = 0; i < pattern_count; i++) {
    s3m_pat_pointers[i] = fgetw(file);
    optional_printf("Pattern %zu:\n", i);
    s3m_pat_pointers[i] = (u16)convert_from_parapointer(s3m_pat_pointers[i]);
  }
}

void check_s3m_channels(void) {
  usize i = 0, channel = 0;

  while (i++ < S3M_MAXCHN) {
    channel = s3m_song_header[S3M_CHANNELARRAYPOS + (i - 1)];

    if (channel == S3MCHN_DISABLED)
      break;

    if (i > STM_MAXCHN) {
      warning_puts("There's more than 4 channels, they'll be truncated.");
      break;
    }

    if (channel & S3MCHN_MUTE)
      warning_printf("Channel %u is muted which is unsupported, the notes will be converted anyway.\n", i);

    if (channel >= S3MCHN_ADLIBMEL1 && channel <= S3MCHN_ADLIBHATDRUM) {
      warning_printf(
          "Adlib channel detected (channel %u), the notes will be converted as is without the instrument data.\n", i);
    }
  }
}

// s3m_song_header is expected to be filled beforehand
void convert_song_header_s3mtostm(void) {
  const u8 song_flags = s3m_song_header[38], initial_speed = s3m_song_header[49], master_volume = s3m_song_header[51],
           global_volume = s3m_song_header[48];

  strncpy((char*)stm_song_header, (char*)s3m_song_header, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    warning_puts("The Amiga frequency limit option is not supported in Scream Tracker 2.");

  if (song_flags & S3M_ST2TEMPO) {
    stm_song_header[32] = initial_speed;
  } else {
    // TODO: deal with speed factor
    stm_song_header[32] = initial_speed << 4;
  }

  if (master_volume & 128)
    warning_puts("Do not expect the song to play in stereo.");

  stm_song_header[34] = global_volume;

  stm_song_header[33] = pattern_count;
}

void convert_song_header_s3mtostx(void) {
  const u8 song_flags = s3m_song_header[38], initial_speed = s3m_song_header[49], master_volume = s3m_song_header[51],
           global_volume = s3m_song_header[48];

  strncpy((char*)stx_song_header, (char*)s3m_song_header, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    warning_puts("Ignoring Amiga frequency limit");
  if (song_flags & S3M_ST2TEMPO) {
    stx_song_header[43] = initial_speed;
  } else {
    // TODO: deal with speed factor
    stx_song_header[43] = initial_speed << 4;
  }

  if (master_volume & 128)
    warning_puts("Do not expect the song to play in stereo.");

  stx_song_header[42] = global_volume;

  stx_song_header[48] = pattern_count;

  stx_song_header[50] = sample_count;

  stx_song_header[52] = order_count;
}

void convert_song_orders_s3mtostm(usize length) {
  usize i = 0;

  memset(stm_order_list, STM_ORDER_END, STM_ORDER_LIST_SIZE);

  do {
    if (i >= length)
      break;

    // the original order list is already parsed out, so don't worry about it here.
    //stm_order_list[i] = (s3m_order_array[i] >= STM_MAXPAT) ? STM_ORDER_END : s3m_order_array[i];

    stm_order_list[i] = s3m_order_array[i];
  } while (++i < STM_ORDER_LIST_SIZE);
}

void convert_song_orders_s3mtostx(usize length, u8* order_list) {
  usize i = 0;

  if (!order_list)
    return;

  do {
    // I have no idea why STX specifically has its order list like this...
    order_list[i * STX_ORDERMULTIPLIER] = s3m_order_array[i];
  } while (++i < length);
}

void grab_sample_data(FILE* file, usize position) {
  if (!file || feof(file) || ferror(file))
    return;
  fseek(file, (long)position, SEEK_SET);
  (void)!fread(s3m_inst_header, sizeof(u8), 80, file);
}

void handle_sample_name_s3m2stm(void) {
  usize i = 0, random = 0;
  const u32 crc = crc32(s3m_inst_header, 80);

  srand(crc);

  random = (usize)rand();

  if (s3m_inst_header[1] != 0) {
    memcpy((char*)stm_sample_header, (char*)&s3m_inst_header[1], 12);
  } else if (s3m_inst_header[48] != 0) {
    memcpy((char*)stm_sample_header, (char*)&s3m_inst_header[48], 12);

    if (!main_context.sanitize_sample_names)
      return;

    // sanitization for 8.3 filenames
    for (i = 0; i < 8; i++) {
      if (stm_sample_header[i] == 0x20) {
        // non-breaking space, it'll look strange in OpenMPT but oh well.
        stm_sample_header[i] = 0xFF;
      } else if (stm_sample_header[i] < 0x20 || stm_sample_header[i] >= 0x7E) {
        // me exploiting the fact that Scream Tracker is coded in C, hehe!
        stm_sample_header[i] = 0x00;
      }
    }
    stm_sample_header[8] = '.';
    snprintf((char*)&stm_sample_header[9], 4, "%03zu", (usize)crc % 999);
  } else {
    if (!main_context.sanitize_sample_names) {
      memset(stm_sample_header, 0, 12);
      return;
    };
    snprintf((char*)&stm_sample_header[0], 12, "X%06zuX", (usize)random);
    stm_sample_header[8] = '.';
    snprintf((char*)&stm_sample_header[9], 4, "%03zu", (usize)crc % 999);
  }
}

void convert_s3m_intstrument_header_s3mtostm(void) {
  const usize type = s3m_inst_header[0], flags = s3m_inst_header[31];

  switch (type) {
    case S3MSMPTYPE_MSG:
    generateblanksample:
      // instrument name
      if (s3m_inst_header[1] != 0)
        memcpy((char*)stm_sample_header, (char*)&s3m_inst_header[1], 12);
      else if (s3m_inst_header[48] != 0)
        strncpy((char*)stm_sample_header, (char*)&s3m_inst_header[48], 12);
      else
        memset(stm_sample_header, 0, 12);

      // instrument disk
      stm_sample_header[13] = 0;

      // lengths
      stm_sample_header[17] = 0, stm_sample_header[16] = 0;

      // loop points
      stm_sample_header[19] = 0, stm_sample_header[18] = 0;
      stm_sample_header[21] = 0xFF, stm_sample_header[20] = 0xFF;

      // volume
      stm_sample_header[22] = 0;

      // c2spd
      stm_sample_header[25] = 0x21, stm_sample_header[24] = 0;
      break;

    case S3MSMPTYPE_SMP:
      handle_sample_name_s3m2stm();

      // instrument disk
      stm_sample_header[13] = 0;

      // lengths
      stm_sample_header[17] = s3m_inst_header[17], stm_sample_header[16] = s3m_inst_header[16];

      // loop points
      if (flags & S3MSMP_LOOP) {
        stm_sample_header[19] = s3m_inst_header[21], stm_sample_header[18] = s3m_inst_header[20];

        stm_sample_header[21] = s3m_inst_header[25], stm_sample_header[20] = s3m_inst_header[24];
      } else {
        stm_sample_header[19] = 0, stm_sample_header[18] = 0;
        stm_sample_header[21] = 0xFF, stm_sample_header[20] = 0xFF;
      }

      // volume
      stm_sample_header[22] = s3m_inst_header[28];

      // c2spd
      stm_sample_header[25] = s3m_inst_header[33], stm_sample_header[24] = s3m_inst_header[32];
      break;

    default:
      warning_puts("Adlib instrument is not supported, only converting sample name.");
      goto generateblanksample;
      break;
  }
}

u32 grab_s3m_pcm_pointer(void) {
  const u32 parapointer = (s3m_inst_header[13] << 16 | s3m_inst_header[15] << 8 | s3m_inst_header[14]) << 4;
  optional_printf("PCM Parapointer: %lX\n", parapointer);
  return parapointer;
}

u16 grab_s3m_pcm_len(void) {
  const u16 length = s3m_inst_header[17] << 8 | s3m_inst_header[16];

  if ((s3m_inst_header[19] << 8 | s3m_inst_header[18]) != 0)
    warning_puts("the sample is too long, only converting the first 64kb of it.\n");

  return length;
}

void generate_blank_stm_instrument(void) {
  memset(stm_sample_header, 0, 12);

  // instrument disk
  stm_sample_header[13] = 0;

  // lengths
  stm_sample_header[17] = 0, stm_sample_header[16] = 0;

  // loop points
  stm_sample_header[19] = 0, stm_sample_header[18] = 0;
  stm_sample_header[21] = 0xFF, stm_sample_header[20] = 0xFF;

  // volume
  stm_sample_header[22] = 0;

  // c2spd
  stm_sample_header[25] = 0x21, stm_sample_header[24] = 0;
}
