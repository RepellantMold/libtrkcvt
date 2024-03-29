#ifndef __MAIN_H
#define __MAIN_H
#include "envcheck.h"
#include "ext.h"
#include "s3m.h"
#include "stm.h"

u8 order_count;
u8 sample_count;
u8 pattern_count;

void eprintf(const char* format, ...);
void eputs(const char* msg);

int convert_s3m_to_stm(FILE *S3Mfile, FILE *STMfile);
int convert_s3m_to_stx(FILE *S3Mfile, FILE *STXfile);

#endif
