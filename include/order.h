#ifndef __SONG_HEADER_H
#define __SONG_HEADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ext.h"

void grab_s3m_orders(FILE* file);
void convert_song_orders_s3mtostm(usize length);
void convert_song_orders_s3mtostx(usize length, u8* order_list);

#endif
