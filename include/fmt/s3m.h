#ifndef __S3M_H
#define __S3M_H
#include <stdio.h>
#include <stdlib.h>

#include "../ext.h"

#define S3M_MAXPAT          253
#define S3M_MAXCHN          32
#define S3M_MAXSMP          99

#define S3M_ORDERPOS        96
#define S3M_ORDER_LIST_SIZE 256
#define S3M_ORDER_MARKER    0xFE
#define S3M_ORDER_END       0xFF
#define S3M_CHANNELARRAYPOS 64

enum s3m_song_flags {
  S3M_ST2VIB = 1,
  S3M_ST2TEMPO = 2,
  S3M_AMIGASLIDE = 4,
  S3M_VOL0OPT = 8,
  S3M_AMIGAFREQLIMITS = 16,
  S3M_ENABLEFILTER = 64,
  S3M_CUSTOMDATA = 128
};

enum s3m_sample_type { S3MSMPTYPE_MSG = 0, S3MSMPTYPE_SMP = 1, S3MSMPTYPE_ADLIBMEL = 2, S3MSMPTYPE_ADLIBDRUM = 3 };

enum s3m_sample_flags { S3MSMP_LOOP = 1, S3MSMP_STEREO = 2, S3MSMP_16BIT = 4 };

enum s3m_channel_type {
  S3MCHN_LEFT1 = 0,
  S3MCHN_LEFT2 = 1,
  S3MCHN_LEFT3 = 2,
  S3MCHN_LEFT4 = 3,
  S3MCHN_LEFT5 = 4,
  S3MCHN_LEFT6 = 5,
  S3MCHN_LEFT7 = 6,
  S3MCHN_LEFT8 = 7,

  S3MCHN_RIGHT1 = 8,
  S3MCHN_RIGHT2 = 9,
  S3MCHN_RIGHT3 = 10,
  S3MCHN_RIGHT4 = 11,
  S3MCHN_RIGHT5 = 12,
  S3MCHN_RIGHT6 = 13,
  S3MCHN_RIGHT7 = 14,
  S3MCHN_RIGHT8 = 15,

  S3MCHN_ADLIBMEL1 = 16,
  S3MCHN_ADLIBMEL2 = 17,
  S3MCHN_ADLIBMEL3 = 18,
  S3MCHN_ADLIBMEL4 = 19,
  S3MCHN_ADLIBMEL5 = 20,
  S3MCHN_ADLIBMEL6 = 21,
  S3MCHN_ADLIBMEL7 = 22,
  S3MCHN_ADLIBMEL8 = 23,
  S3MCHN_ADLIBMEL9 = 24,

  S3MCHN_ADLIBKICKDRUM = 25,
  S3MCHN_ADLIBSNAREDRUM = 26,
  S3MCHN_ADLIBTOMDRUM = 27,
  S3MCHN_ADLIBTOPDRUM = 28,
  S3MCHN_ADLIBHATDRUM = 29,

  S3MCHN_MUTE = 128,

  S3MCHN_DISABLED = 255
};

/* 320 bytes per channel
64 sequential rows per channel
5 bytes per row */
#define S3M_UNPACKED_PATTERN_SIZE ((5 * 32) * 64)

typedef struct {
  char title[28];
  char dos_eof;
  char type;
  u16 reserved1;
  u16 total_orders;
  u16 total_instruments;
  u16 total_patterns;
  u16 flags;
  u16 created_with_tracker_version;
  u16 file_format_information;
  char scrm[4];
  u8 global_volume;
  u8 initial_speed;
  u8 initial_tempo;
  u8 master_volume;
  u8 ultraclick_removal;
  u8 default_panning;
  char reserved2[8];
  u16 special;
  u8 channel_settings[32];
} s3m_song_header_t;

typedef struct {
  enum s3m_sample_type type;
  char filename[12];

  union {
    struct {
      u8 high;
      u8 low1;
      u8 low2;
    } bytes;

    struct {
      u8 high_byte;
      u16 low;
    } words;

    u32 full; // technically wrong but there's no 24-bit type....
  } memseg;

  union {
    struct {
      u16 low;
      u16 high;
    } words;

    u32 full;
  } length;

  union {
    struct {
      u16 low;
      u16 high;
    } words;

    u32 full;
  } loop_start;

  union {
    struct {
      u16 low;
      u16 high;
    } words;

    u32 full;
  } loop_end;

  u8 default_volume;
  char reserved1;
  u8 packing;
  u8 flags;

  union {
    struct {
      u16 low;
      u16 high;
    } words;

    u32 full;
  } c_speed;

  char reserved2[12];
  char name[28];
  char scrs[4];
} s3m_instrument_header_t;

extern u8 s3m_order_array[S3M_ORDER_LIST_SIZE];

extern s3m_song_header_t s3m_song_header;
extern s3m_instrument_header_t s3m_inst_header;

extern u16 s3m_cwtv;

extern u16 s3m_inst_pointers[S3M_MAXSMP];
extern u16 s3m_pat_pointers[S3M_MAXPAT];
extern u32 s3m_pcm_pointers[S3M_MAXSMP];
extern u16 s3m_pcm_lens[S3M_MAXSMP];

struct S3MEvent {
  u8 note, ins, vol, eff, prm;
};

#endif
