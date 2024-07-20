/* some ANSI C "extensions" to help code readability
based on https://github.com/vkclarke/ansi-c-extensions */
#ifndef __ANSI_EXT_H
#define __ANSI_EXT_H
#include <stddef.h>
#include <stdio.h>
#include "envcheck.h"

typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short int u16;
typedef signed short int i16;
typedef unsigned long int u32;
typedef signed long int i32;
typedef size_t usize;

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
typedef _Bool bool;
#define true 1
#define false 0
#endif
#else
typedef enum { false, true } bool;
#endif

#define GET_BIT(byte, bit) ((byte) & (1 << (bit)))

/* do {} forever / forever {} */
#define forever while (1)

/* loop {} */
#define loop    for (;;)

#endif
