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

/* do {} forever / forever {} */
#define forever while (1)

/* loop {} */
#define loop    for (;;)

#endif
