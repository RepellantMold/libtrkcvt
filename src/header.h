#ifndef __HEADER_H
#define __HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext.h"

void show_s3m_song_header(void);
void show_s3m_inst_header(void);

void check_s3m_channels(void);

void generate_blank_stm_instrument(void);

void grab_s3m_orders(FILE* file);
void grab_s3m_parapointers(FILE* file);
void grab_sample_data(FILE* file, usize position);
u32 grab_s3m_pcm_pointer(void);
u16 grab_s3m_pcm_len(void);

void convert_song_header_s3mtostm(void);
void convert_song_header_s3mtostx(void);
void convert_song_orders_s3mtostm(usize length);
void convert_song_orders_s3mtostx(usize length, u8* order_list);
void convert_s3m_intstrument_header_s3mtostm(void);

void handle_sample_name_s3m2stm(void);

static u16 fgetw(FILE* fp);

#endif
