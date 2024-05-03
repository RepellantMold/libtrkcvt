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

extern u8 s3m_order_array[S3M_ORDER_LIST_SIZE];

extern u8 s3m_song_header[96];
extern u8 s3m_inst_header[80];

extern u16 s3m_cwtv;

extern u16 s3m_inst_pointers[S3M_MAXSMP];
extern u16 s3m_pat_pointers[S3M_MAXPAT];
extern u32 s3m_pcm_pointers[S3M_MAXSMP];
extern u16 s3m_pcm_lens[S3M_MAXSMP];

struct S3MEvent {
  u8 note, ins, vol, eff, prm;
};

static struct S3MEvent s3m_unpacked_pattern[64][32] = {{{0xFF, 0x00, 0xFF, 0x00, 0x00}}};

#endif
