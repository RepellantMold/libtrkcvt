#ifndef __S3M_H
#define __S3M_H

#define S3M_MAXCHN 32
#define S3M_MAXSMP 99
#define S3M_ORDER_MARKER 0xFE
#define S3M_ORDER_END 0xFF

/* 320 bytes per channel
 * 64 sequential rows per channel
 * 5 bytes per row
 */
#define S3M_UNPACKED_PATTERN_SIZE ((5*32)*64)

#endif