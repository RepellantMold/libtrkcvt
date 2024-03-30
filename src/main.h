#ifndef __MAIN_H
#define __MAIN_H
#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

u8 order_count;
u8 sample_count;
u8 pattern_count;
bool verbose_mode = false;

void eprintf(const char* format, ...);
void eputs(const char* msg);

void optional_printf(const char* format, ...);
void optional_puts(const char* msg);

void warning_puts(const char* msg);
void warning_printf(const char* format, ...);

int convert_s3m_to_stm(FILE *S3Mfile, FILE *STMfile, bool verbose_mode);
int convert_s3m_to_stx(FILE *S3Mfile, FILE *STXfile, bool verbose_mode);

#endif
