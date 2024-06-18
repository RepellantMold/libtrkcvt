#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"
#include "fmt/stx.h"

#include "file.h"
#include "log.h"
#include "parapnt.h"
#include "sample.h"
#include "song_header.h"


#include "crc.h"

void show_s3m_song_header(void) {
  const u8 global_volume = s3m_song_header.global_volume, initial_speed = s3m_song_header.initial_speed,
           initial_tempo = s3m_song_header.initial_tempo, song_flags = (u8)s3m_song_header.flags;

  printf("Song title: %s\n"
         "Global volume: %u\n"
         "Initial speed/tempo: %02X/%02X\n"
         "Song flags: %02X\n",
         s3m_song_header.title, global_volume, initial_speed, initial_tempo, song_flags);
}

void grab_s3m_orders(FILE* file) {
  register usize i = 0, count = 0;

  if (!file || feof(file) || ferror(file))
    return;

  fseek(file, S3M_ORDERPOS, SEEK_SET);

  (void)!fread(s3m_order_array, sizeof(u8), original_order_count, file);

  /* see section "2.6 Load Order Data" from "FireLight S3M Player Tutorial.txt" */
  for (count = 0; count < original_order_count; ++count) {
    if (s3m_order_array[count] < S3M_ORDER_MARKER) {
      s3m_order_array[i] = s3m_order_array[count];
      if (s3m_order_array[count] > pattern_count)
        pattern_count = s3m_order_array[count];
      print_diagnostic("Order %zu -> %zu", count, i++);
    }
  }
  order_count = (u8)i;
}

void check_s3m_channels(void) {
  register usize i = 0, channel = 0;

  while (i++ < S3M_MAXCHN) {
    channel = s3m_song_header.channel_settings[(i - 1)];

    if (channel == S3MCHN_DISABLED)
      break;

    if (i > STM_MAXCHN) {
      print_warning("There's more than 4 channels, they'll be truncated.");
      break;
    }

    if (channel & S3MCHN_MUTE)
      print_warning("Channel %u is muted which is unsupported, the notes will be converted anyway.", i);

    if (channel >= S3MCHN_ADLIBMEL1 && channel <= S3MCHN_ADLIBHATDRUM) {
      print_warning(
          "Adlib channel detected (channel %u), the notes will be converted as is without the instrument data.", i);
    }
  }
}

void grab_s3m_song_header(FILE* S3Mfile) {
  const bool verbose = main_context.flags.verbose_mode;
  if (!S3Mfile || feof(S3Mfile) || ferror(S3Mfile))
    return;

  fread(s3m_song_header.title, 28, 1, S3Mfile);
  s3m_song_header.dos_eof = fgetb(S3Mfile);
  s3m_song_header.type = fgetb(S3Mfile);
  s3m_song_header.reserved1 = fgetw(S3Mfile);
  s3m_song_header.total_orders = fgetw(S3Mfile);
  s3m_song_header.total_instruments = fgetw(S3Mfile);
  s3m_song_header.total_patterns = fgetw(S3Mfile);
  s3m_song_header.flags = fgetw(S3Mfile);
  s3m_song_header.created_with_tracker_version = fgetw(S3Mfile);
  s3m_song_header.file_format_information = fgetw(S3Mfile);
  fread(s3m_song_header.scrm, 4, 1, S3Mfile);
  s3m_song_header.global_volume = fgetb(S3Mfile);
  s3m_song_header.initial_speed = fgetb(S3Mfile);
  s3m_song_header.initial_tempo = fgetb(S3Mfile);
  s3m_song_header.master_volume = fgetb(S3Mfile);
  s3m_song_header.ultraclick_removal = fgetb(S3Mfile);
  s3m_song_header.default_panning = fgetb(S3Mfile);
  fread(s3m_song_header.reserved2, 8, 1, S3Mfile);
  s3m_song_header.special = fgetw(S3Mfile);
  s3m_cwtv = s3m_song_header.created_with_tracker_version;
  original_order_count = (u8)s3m_song_header.total_orders;
  sample_count = (u8)s3m_song_header.total_instruments;
  if (verbose)
    show_s3m_song_header();

  check_s3m_channels();
}

void write_stm_song_header(FILE* STMfile) {
  if (!STMfile || feof(STMfile) || ferror(STMfile))
    return;
  memcpy(stm_song_header.tracker, "!Scrvrt!", 8);
  stm_song_header.dos_eof = 0x1A;
  stm_song_header.type = 0x02;
  stm_song_header.version.bytes.major = 0x02;
  stm_song_header.version.bytes.minor = 0x15;

  fwrite(stm_song_header.title, 20, 1, STMfile);
  fwrite(stm_song_header.tracker, 8, 1, STMfile);
  fputc(stm_song_header.dos_eof, STMfile);
  fputc(stm_song_header.type, STMfile);
  fputc(stm_song_header.version.bytes.major, STMfile);
  fputc(stm_song_header.version.bytes.minor, STMfile);
  fputc(stm_song_header.initial_tempo, STMfile);
  fputc(stm_song_header.total_patterns, STMfile);
  fputc(stm_song_header.global_volume, STMfile);
  fwrite(stm_song_header.reserved1, 13, 1, STMfile);
}

void write_stx_song_header(FILE* STXfile) {
  if (!STXfile || feof(STXfile) || ferror(STXfile))
    return;

  memcpy(stx_song_header.tracker, "!Scream!", 8);
  stx_song_header.misc.dos_eof = 0x1A;
  stx_song_header.reserved2 = 1;
  memcpy(stx_song_header.scrm, "SCRM", 4);

  fwrite(stx_song_header.title, 20, 1, STXfile);
  fwrite(stx_song_header.tracker, 8, 1, STXfile);
  fputw(stx_song_header.misc.first_pattern_size, STXfile);
  fputw(stx_song_header.reserved1, STXfile);
  fputw(stx_song_header.pattern_table_parapointer, STXfile);
  fputw(stx_song_header.instrument_table_parapointer, STXfile);
  fputw(stx_song_header.channel_table_parapointer, STXfile);
  fputl(stx_song_header.reserved2, STXfile);
  fputc(stx_song_header.global_volume, STXfile);
  fputc(stx_song_header.initial_tempo, STXfile);
  fputc(stx_song_header.reserved3, STXfile);
  fputw(stx_song_header.total_patterns, STXfile);
  fputw(stx_song_header.total_instruments, STXfile);
  fputw(stx_song_header.total_orders, STXfile);
  fwrite(stx_song_header.reserved4, 6, 1, STXfile);
  fwrite(stx_song_header.scrm, 4, 1, STXfile);
}

void convert_song_header_s3mtostm(void) {
  const u8 song_flags = (u8)s3m_song_header.flags, initial_speed = s3m_song_header.initial_speed,
           master_volume = s3m_song_header.master_volume, global_volume = s3m_song_header.global_volume;

  strncpy((char*)stm_song_header.title, (char*)s3m_song_header.title, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    print_warning("The Amiga frequency limit option is not supported in Scream Tracker 2.");

  if (song_flags & S3M_ST2TEMPO) {
    stm_song_header.initial_tempo = initial_speed;
  } else {
    /* TODO: deal with speed factor */
    stm_song_header.initial_tempo = (u8)(initial_speed << 4);
  }

  if (master_volume & 128)
    print_warning("Do not expect the song to play in stereo.");

  stm_song_header.global_volume = global_volume;

  stm_song_header.total_patterns = pattern_count;
}

void convert_song_header_s3mtostx(void) {
  const u8 song_flags = (u8)s3m_song_header.flags, initial_speed = s3m_song_header.initial_speed,
           master_volume = s3m_song_header.master_volume, global_volume = s3m_song_header.global_volume;

  strncpy((char*)stx_song_header.title, (char*)s3m_song_header.title, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    print_warning("Ignoring Amiga frequency limit");
  if (song_flags & S3M_ST2TEMPO) {
    stx_song_header.initial_tempo = initial_speed;
  } else {
    /* TODO: deal with speed factor */
    stx_song_header.initial_tempo = (u8)(initial_speed << 4);
  }

  if (master_volume & 128)
    print_warning("Do not expect the song to play in stereo.");

  stx_song_header.global_volume = global_volume;

  stx_song_header.total_patterns = pattern_count;
  stx_song_header.total_instruments = sample_count;
  stx_song_header.total_orders = order_count;
}

void convert_song_orders_s3mtostm(usize length) {
  register usize i = 0;

  memset(stm_order_list, STM_ORDER_END, STM_ORDER_LIST_SIZE);

  do {
    if (i >= length)
      break;

    stm_order_list[i] = (s3m_order_array[i] > STM_MAXPAT) ? STM_ORDER_END : s3m_order_array[i];
  } while (++i < STM_ORDER_LIST_SIZE);
}

void convert_song_orders_s3mtostx(usize length, u8* order_list) {
  register usize i = 0;

  if (!order_list)
    return;

  do {
    /* I have no idea why STX specifically has its order list like this... */
    order_list[i * STX_ORDERMULTIPLIER] = s3m_order_array[i];
  } while (++i < length);
}
