#ifndef __STM_H
#define __STM_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "../ext.h"

#define STM_MAXCHN          4
#define STM_MAXSMP          31

/* Strangely, in ST2, you can actually add orders above 63 (it even writes the bytes to the file),
but the pattern editor does not go above 63 and no players will accept this so... */
#define STM_MAXPAT          63

#define STM_ORDER_END       99
#define STM_ORDER_LIST_SIZE 128

#define STM_PATSIZE         ((4 * 4) * 64)

extern u8 stm_song_header[48];

extern u8 stm_sample_header[32];

extern u8 stm_sample_data[USHRT_MAX];

extern u8 stm_order_list[STM_ORDER_LIST_SIZE];

extern u8 stm_pattern[64][4][4];

extern u16 stm_pcm_pointers[STM_MAXSMP];

#endif
