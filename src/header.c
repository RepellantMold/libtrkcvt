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
#include "header.h"
#include "log.h"
#include "parapnt.h"
#include "sample.h"

#include "crc.h"

void show_s3m_song_header(void) {
  const u8 global_volume = s3m_song_header.global_volume, initial_speed = s3m_song_header.initial_speed,
           initial_tempo = s3m_song_header.initial_tempo, song_flags = s3m_song_header.flags;

  printf("Song title: %s\n"
         "Global volume: %u\n"
         "Initial speed/tempo: %02X/%02X\n"
         "Song flags: %02X\n",
         s3m_song_header.title, global_volume, initial_speed, initial_tempo, song_flags);
}

void show_s3m_inst_header(void) {
  const u8 default_volume = s3m_inst_header.default_volume, sample_flags = s3m_inst_header.flags;
  const u16 c_frequency = s3m_inst_header.c_speed.full, length = s3m_inst_header.length.full,
            loop_start = s3m_inst_header.loop_start.full, loop_end = s3m_inst_header.loop_end.full;

  printf("Sample name/filename: %.28s/%.12s\n"
         "Default volume: %02u\n"
         "Sample flags: %01X\n"
         "C frequency: %06u\n"
         "Length/Loop start/end: %06u/%06u/%06u\n",
         s3m_inst_header.name, s3m_inst_header.filename, default_volume, sample_flags, c_frequency, length, loop_start,
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
      print_diagnostic("Order %zu -> %zu", count, i++);
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
    print_diagnostic("Sample %zu:", i);
    s3m_inst_pointers[i] = (u16)convert_from_parapointer(s3m_inst_pointers[i]);
  }

  for (i = 0; i < pattern_count; i++) {
    s3m_pat_pointers[i] = fgetw(file);
    print_diagnostic("Pattern %zu:", i);
    s3m_pat_pointers[i] = (u16)convert_from_parapointer(s3m_pat_pointers[i]);
  }
}

void check_s3m_channels(void) {
  usize i = 0, channel = 0;

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
  const bool verbose = main_context.verbose_mode;
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
  original_order_count = s3m_song_header.total_orders;
  sample_count = s3m_song_header.total_instruments;
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

  memcpy(stm_song_header.tracker, "!Scream!", 8);
  stx_song_header.misc.dos_eof = 0x1A;
  stx_song_header.reserved2 = 1;
  memcpy(stm_song_header.scrm, "SCRM", 4);

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

void write_stm_instrument_header(FILE* STMfile, stm_instrument_header_t* stm_instrument_header) {
  fwrite(stm_instrument_header->filename, 12, 1, STMfile);
  fputc(stm_instrument_header->zero, STMfile);
  fputc(stm_instrument_header->disk, STMfile);
  fputw(stm_instrument_header->parapointer, STMfile);
  fputw(stm_instrument_header->length, STMfile);
  fputw(stm_instrument_header->loop_start, STMfile);
  fputw(stm_instrument_header->loop_end, STMfile);
  fputc(stm_instrument_header->default_volume, STMfile);
  fputc(stm_instrument_header->reserved1, STMfile);
  fputw(stm_instrument_header->c_speed, STMfile);
  fwrite(stm_instrument_header->reserved2, 6, 1, STMfile);
}

void write_stx_instrument_header(FILE* STXfile) {
  fputc(s3m_inst_header.type, STXfile);
  fwrite(s3m_inst_header.filename, 12, 1, STXfile);
  fputc(s3m_inst_header.memseg.bytes.high, STXfile);
  fputc(s3m_inst_header.memseg.bytes.low1, STXfile);
  fputc(s3m_inst_header.memseg.bytes.low2, STXfile);
  fputw(s3m_inst_header.length.words.low, STXfile);
  fputw(s3m_inst_header.length.words.high, STXfile);
  fputw(s3m_inst_header.loop_start.words.low, STXfile);
  fputw(s3m_inst_header.loop_start.words.high, STXfile);
  fputw(s3m_inst_header.loop_end.words.low, STXfile);
  fputw(s3m_inst_header.loop_end.words.high, STXfile);
  fputc(s3m_inst_header.default_volume, STXfile);
  fputc(s3m_inst_header.reserved1, STXfile);
  fputc(s3m_inst_header.packing, STXfile);
  fputc(s3m_inst_header.flags, STXfile);
  fputw(s3m_inst_header.c_speed.words.low, STXfile);
  fputw(s3m_inst_header.c_speed.words.high, STXfile);
  fwrite(s3m_inst_header.reserved2, 12, 1, STXfile);
  fwrite(s3m_inst_header.name, 28, 1, STXfile);
  fwrite(s3m_inst_header.scrs, 4, 1, STXfile);
}

// s3m_song_header is expected to be filled beforehand
void convert_song_header_s3mtostm(void) {
  const u8 song_flags = s3m_song_header.flags, initial_speed = s3m_song_header.initial_speed,
           master_volume = s3m_song_header.master_volume, global_volume = s3m_song_header.global_volume;

  strncpy((char*)stm_song_header.title, (char*)s3m_song_header.title, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    print_warning("The Amiga frequency limit option is not supported in Scream Tracker 2.");

  if (song_flags & S3M_ST2TEMPO) {
    stm_song_header.initial_tempo = initial_speed;
  } else {
    // TODO: deal with speed factor
    stm_song_header.initial_tempo = initial_speed << 4;
  }

  if (master_volume & 128)
    print_warning("Do not expect the song to play in stereo.");

  stm_song_header.global_volume = global_volume;

  stm_song_header.total_patterns = pattern_count;
}

void convert_song_header_s3mtostx(void) {
  const u8 song_flags = s3m_song_header.flags, initial_speed = s3m_song_header.initial_speed,
           master_volume = s3m_song_header.master_volume, global_volume = s3m_song_header.global_volume;

  strncpy((char*)stx_song_header.title, (char*)s3m_song_header.title, 19);

  if (song_flags & S3M_AMIGAFREQLIMITS)
    print_warning("Ignoring Amiga frequency limit");
  if (song_flags & S3M_ST2TEMPO) {
    stx_song_header.initial_tempo = initial_speed;
  } else {
    // TODO: deal with speed factor
    stx_song_header.initial_tempo = initial_speed << 4;
  }

  if (master_volume & 128)
    print_warning("Do not expect the song to play in stereo.");

  stx_song_header.global_volume = global_volume;

  stx_song_header.total_patterns = pattern_count;
  stx_song_header.total_instruments = sample_count;
  stx_song_header.total_orders = order_count;
}

void convert_song_orders_s3mtostm(usize length) {
  usize i = 0;

  memset(stm_order_list, STM_ORDER_END, STM_ORDER_LIST_SIZE);

  do {
    if (i >= length)
      break;

    stm_order_list[i] = (s3m_order_array[i] > STM_MAXPAT) ? STM_ORDER_END : s3m_order_array[i];
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

void grab_s3m_isntrument_header_data(FILE* file, usize position) {
  if (!file || feof(file) || ferror(file))
    return;
  fseek(file, (long)position, SEEK_SET);
  s3m_inst_header.type = fgetb(file);
  fread(s3m_inst_header.filename, 12, 1, file);
  s3m_inst_header.memseg.bytes.high = fgetb(file);
  s3m_inst_header.memseg.bytes.low1 = fgetb(file);
  s3m_inst_header.memseg.bytes.low2 = fgetb(file);
  s3m_inst_header.memseg.full = (s3m_inst_header.memseg.bytes.high << 16 | s3m_inst_header.memseg.bytes.low2 << 8
                                 | s3m_inst_header.memseg.bytes.low1);
  s3m_inst_header.length.words.low = fgetw(file);
  s3m_inst_header.length.words.high = fgetw(file);
  s3m_inst_header.loop_start.words.low = fgetw(file);
  s3m_inst_header.loop_start.words.high = fgetw(file);
  s3m_inst_header.loop_end.words.low = fgetw(file);
  s3m_inst_header.loop_end.words.high = fgetw(file);
  s3m_inst_header.default_volume = fgetb(file);
  s3m_inst_header.reserved1 = fgetb(file);
  s3m_inst_header.packing = fgetb(file);
  s3m_inst_header.flags = fgetb(file);
  s3m_inst_header.c_speed.words.low = fgetw(file);
  s3m_inst_header.c_speed.words.high = fgetw(file);
  fread(s3m_inst_header.reserved2, 12, 1, file);
  fread(s3m_inst_header.name, 28, 1, file);
  fread(s3m_inst_header.scrs, 4, 1, file);
}

void handle_sample_name_s3m2stm(stm_instrument_header_t* stm_sample_header) {
  usize i = 0, random = 0;
  const u32 crc = crc32((u8*)&s3m_inst_header, sizeof(s3m_inst_header));

  srand(crc);

  random = (usize)rand();

  if (s3m_inst_header.filename[0] != 0) {
    memcpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.filename, 12);
  } else if (s3m_inst_header.name[0] != 0) {
    strncpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.name, 12);

    if (!main_context.sanitize_sample_names)
      return;

    // sanitization for 8.3 filenames
    for (i = 0; i < 8; i++) {
      u8 c = stm_sample_header->filename[i];
      if (c == 0x20) {
        // non-breaking space, it'll look strange in OpenMPT but oh well.
        c = 0xFF;
      } else if (c < 0x20 || c > 0x7E) {
        // me exploiting the fact that Scream Tracker is coded in C, hehe!
        c = 0x00;
      }

      stm_sample_header->filename[i] = c;
    }
    stm_sample_header->filename[8] = '.';
    snprintf((char*)&stm_sample_header->filename[9], 4, "%03zu", (usize)crc % 999);
  } else {
    if (!main_context.sanitize_sample_names) {
      memset(stm_sample_header->filename, 0, 12);
      return;
    };
    snprintf((char*)&stm_sample_header->filename[0], 13, "X%06zuX.%03zu", (usize)random, (usize)crc % 999);
  }
}

void convert_s3m_intstrument_header_s3mtostm(stm_instrument_header_t* stm_sample_header) {
  const usize type = s3m_inst_header.type, flags = s3m_inst_header.flags;

  switch (type) {
    case S3MSMPTYPE_MSG:
    generateblanksample:
      if (s3m_inst_header.filename[0] != 0)
        memcpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.filename, 12);
      else if (s3m_inst_header.name[0] != 0)
        strncpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.name, 12);
      else
        memset(stm_sample_header->filename, 0, 12);

      stm_sample_header->disk = 0;

      stm_sample_header->length = 0;
      stm_sample_header->loop_start = 0;
      stm_sample_header->loop_end = 0xFFFF;

      stm_sample_header->default_volume = 0;

      stm_sample_header->c_speed = 8448;
      break;

    case S3MSMPTYPE_SMP:
      handle_sample_name_s3m2stm(stm_sample_header);

      stm_sample_header->disk = 0;

      stm_sample_header->length = s3m_inst_header.length.words.low;
      if (flags & S3MSMP_LOOP) {
        stm_sample_header->loop_start = s3m_inst_header.loop_start.words.low;
        stm_sample_header->loop_end = s3m_inst_header.loop_end.words.low;
      } else {
        stm_sample_header->loop_start = 0;
        stm_sample_header->loop_end = 0xFFFF;
      }

      stm_sample_header->default_volume = s3m_inst_header.default_volume;

      stm_sample_header->c_speed = s3m_inst_header.c_speed.words.low;
      break;

    default:
      print_warning("Adlib instrument is not supported, only converting sample name.");
      goto generateblanksample;
      break;
  }
}

u32 grab_s3m_pcm_pointer(void) {
  //const u32 parapointer = (s3m_inst_header.memseg.bytes.high << 16 | s3m_inst_header.memseg.bytes.low2 << 8 | s3m_inst_header.memseg.bytes.low1) << 4;
  const u32 parapointer = s3m_inst_header.memseg.full << 4;
  print_diagnostic("PCM Parapointer: %lX", parapointer);
  return parapointer;
}

u16 grab_s3m_pcm_len(void) {
  const u16 length = s3m_inst_header.length.words.low;

  if ((s3m_inst_header.length.words.high) != 0)
    print_warning("the sample is too long, only converting the first 64kb of it.");

  return length;
}

void generate_blank_stm_instrument(stm_instrument_header_t* stm_sample_header) {
  memset(stm_sample_header->filename, 0, 12);

  stm_sample_header->disk = 0;

  stm_sample_header->length = 0;
  stm_sample_header->loop_start = 0;
  stm_sample_header->loop_end = 0xFFFF;

  stm_sample_header->default_volume = 0;

  stm_sample_header->c_speed = 8448;
}
