#ifndef __FILE_H
#define __FILE_H

#include <stdio.h>

u8 fgetb(FILE* fp);
u16 fgetw(FILE* fp);
u32 fgetl(FILE* fp);

u16 fputw(u16 value, FILE* fp);
u32 fputl(u32 value, FILE* fp);

#endif
