#ifndef __HEADER_H
#define __HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext.h"

void show_s3m_song_header(void);
void show_s3m_inst_header(void);

void check_s3m_channels(void);

void generate_blank_stm_instrument(stm_instrument_header_t* stm_sample_header);

void grab_s3m_song_header(FILE* S3Mfile);
void grab_s3m_orders(FILE* file);
void grab_s3m_parapointers(FILE* file);
void grab_s3m_isntrument_header_data(FILE* file, usize position);
u32 grab_s3m_pcm_pointer(void);
u16 grab_s3m_pcm_len(void);

void convert_song_header_s3mtostm(void);
void convert_song_header_s3mtostx(void);
void write_stm_song_header(FILE* STMfile);
void write_stx_song_header(FILE* STXfile);
void write_stx_instrument_header(FILE* STXfile);
void convert_song_orders_s3mtostm(usize length);
void convert_song_orders_s3mtostx(usize length, u8* order_list);
void convert_s3m_intstrument_header_s3mtostm(stm_instrument_header_t* stm_sample_header);
void write_stm_instrument_header(FILE* STMfile, stm_instrument_header_t* stm_instrument_header);
void handle_sample_name_s3m2stm(stm_instrument_header_t* stm_sample_header);

#endif
