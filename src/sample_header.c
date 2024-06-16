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
#include "sample_header.h"

#include "crc.h"

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

void grab_s3m_instrument_header_data(FILE* file, usize position) {
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

void convert_s3m_instrument_header_s3mtostm(stm_instrument_header_t* stm_sample_header) {
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
