#ifndef __STM_H
#define __STM_H
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "ext.h"

#define STM_MAXCHN          4
#define STM_MAXSMP          31

/* Strangely, in ST2, you can actually add orders above 63 (it even writes the bytes to the file),
but the pattern editor does not go above 63 and no players will accept this so... */
#define STM_MAXPAT          63

#define STM_ORDER_END       99
#define STM_ORDER_LIST_SIZE 128

#define STM_PATSIZE         ((4 * 4) * 64)

static u8 stm_song_header[48] = {
    // song title (ASCIIZ)
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0',

    // tracker name
    '!', 'S', 'c', 'r', 'v', 'r', 't', '!',

    // DOS EOF
    0x1A,

    // file type (in this case, a module.)
    2,

    // major version, minor version
    2, 21,

    // tempo (default)
    0x60,

    // number of patterns
    0,

    // global volume
    64,

    // reserved (which I, RM, turned into a magic string, you're welcome!)
    'S', 'c', 'r', 'e', 'a', 'm', 'v', 'e', 'r', 't', 'e', 'r', '\0'};

static u8 stm_sample_header[32] = {
    // filename (ASCIIZ)
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

    // reserved
    0,

    // instrument disk
    0,

    /* parapointer to the PCM data
    the official documentation is extremely misleading here since it calls this area reserved!
    "(used as internal segment while playing)" */
    0, 0,

    // length in bytes
    0, 0,

    // loop start
    0, 0,

    // loop end (0xFFFF means no loop)
    0xFF, 0xFF,

    // volume
    64,

    // reserved
    0,

    // speed for C2/Mid-C (calculated as Hz, with a default of 8448 or 8192 depending on version)
    0x00, 0x21,

    // reserved
    0, 0, 0, 0,

    /* reserved, contrary to what the official documentation says...
       "internal segment address/(in modules:)length in paragraphs" */
    0, 0};

static u8 stm_sample_data[USHRT_MAX] = {0};

static u8 stm_order_list[STM_ORDER_LIST_SIZE] = {STM_ORDER_END};

static u8 stm_pattern[64][4][4] = {{{0xFF, 0x01, 0x80, 0x00}}};

static u16 stm_pcm_pointers[STM_MAXSMP] = {0};

#endif
