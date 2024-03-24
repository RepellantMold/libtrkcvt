/* S3M.h
 * Helpful preprocessor things
 */

#ifndef __S3M_H
#define __S3M_H

#define S3M_ORDERPOS 96
#define S3M_MAXPAT 253
#define S3M_MAXCHN 32
#define S3M_MAXSMP 99
#define S3M_ORDER_MARKER 0xFE
#define S3M_ORDER_END 0xFF

enum s3m_song_flags {
    S3M_ST2VIB = 1,
    S3M_ST2TEMPO = 2,
    S3M_AMIGASLIDE = 4,
    S3M_VOL0OPT = 8,
    S3M_AMIGAFREQLIMITS = 16,
    S3M_ENABLEFILTER = 64,
    S3M_CUSTOMDATA = 128
};

enum s3m_sample_flags {
    S3MSMP_LOOP = 1,
    S3MSMP_STEREO = 2,
    S3MSMP_16BIT = 4
};

/* 320 bytes per channel
 * 64 sequential rows per channel
 * 5 bytes per row
 */
#define S3M_UNPACKED_PATTERN_SIZE ((5*32)*64)

u8 s3m_song_header[96] = {0};
u8 s3m_inst_header[80] = {0};

u16 s3m_inst_pointers[S3M_MAXSMP] = {0};
u16 s3m_pat_pointers[S3M_MAXPAT] = {0};

u8 s3m_unpacked_pattern[64][32][5] = {0};

#endif
