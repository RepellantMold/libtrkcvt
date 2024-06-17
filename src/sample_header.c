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
  const u32 c_frequency = s3m_inst_header.c_speed.full, length = s3m_inst_header.length.full,
            loop_start = s3m_inst_header.loop_start.full, loop_end = s3m_inst_header.loop_end.full;

  printf("Sample name/filename: %.28s/%.12s\n"
         "Default volume: %02u\n"
         "Sample flags: %01X\n"
         "C frequency: %06lu\n"
         "Length/Loop start/end: %06lu/%06lu/%06lu\n",
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

void sanitize_sample_name(char* name) {
  usize i = 0, j = 0;
  const u32 crc = crc_sum((u8*)&s3m_inst_header.name, sizeof(s3m_inst_header.name));

  if (!name[0]) {
    sprintf(name, "X%06uX.%03u", (rand() % 999999), (u16)(rand() % 999));
    return;
  }

  for (i = 0; i < strlen(s3m_inst_header.name); i++) {
    if (name[i] == 0x20)
      for (j = i; j < strlen(s3m_inst_header.name) - 1; j++) name[j] = name[j + 1];
  }

  /* be sure that the file names are valid 8.3 names */
  for (i = 0; i < 8; i++) {
    u8 c = name[i];

    switch (c) {
      case 0x20:
      case 0x22:
      case 0x2A:
      case 0x2B:
      case 0x2C:
      case 0x2F:
      case 0x3A:
      case 0x3B:
      case 0x3C:
      case 0x3D:
      case 0x3E:
      case 0x3F:
      case 0x5B:
      case 0x5C:
      case 0x5D:
      case 0x5E:
      case 0x7C: c = 0x5F; break;

      default:
        if (c < 0x20 || c > 0x7E)
          c = 0;
        break;
    }

    name[i] = c;
  }
  name[8] = '.';
  sprintf((char*)&name[9], "%03zu", (usize)crc % 999);
}

void handle_sample_name_s3m2stm(stm_instrument_header_t* stm_sample_header) {
  if (s3m_inst_header.filename[0] != 0) {
    memcpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.filename, 12);
  } else if (s3m_inst_header.name[0] != 0) {
    strncpy((char*)stm_sample_header->filename, (char*)&s3m_inst_header.name, 12);

    if (!main_context.sanitize_sample_names)
      return;

    sanitize_sample_name((char*)stm_sample_header->filename);
  } else {
    if (!main_context.sanitize_sample_names) {
      memset(stm_sample_header->filename, 0, 12);
      return;
    };
    sanitize_sample_name((char*)stm_sample_header->filename);
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
