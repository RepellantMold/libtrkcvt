#ifndef __SONG_HEADER_H
#define __SONG_HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext.h"

void show_s3m_song_header(void);
void grab_s3m_song_header(FILE* S3Mfile);
void grab_s3m_orders(FILE* file);
void check_s3m_channels(void);
void convert_song_header_s3mtostm(void);
void convert_song_header_s3mtostx(void);
void write_stm_song_header(FILE* STMfile);
void write_stx_song_header(FILE* STXfile);
void convert_song_orders_s3mtostm(usize length);
void convert_song_orders_s3mtostx(usize length, u8* order_list);

#endif
