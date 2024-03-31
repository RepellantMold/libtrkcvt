#ifndef __HEADER_H
#define __HEADER_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext.h"

void show_s3m_song_header(void);
void grab_s3m_parapointers(FILE* file);
void convert_song_header(void);
void convert_song_orders(usize length);

typedef struct {
  u8 s3m_order_list[S3M_ORDER_LIST_SIZE];
  u8 stm_order_list[STM_ORDER_LIST_SIZE];
  u8 stx_order_list[S3M_ORDER_LIST_SIZE * STX_ORDERMULTIPLIER];
  u8 order_count;
} Order_Context;

typedef struct {
  u8 s3m_song_header[96];
  u8 s3m_inst_header[80];
  u8 stm_song_header[48];
  u8 stm_inst_header[32];
  u8 stx_song_header[64];
} Header_Context;

u32 crc32_for_byte(u32 byte);
u32 crc32(const unsigned char *input, usize size);

#endif