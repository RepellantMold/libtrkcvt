#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ext.h"

void show_s3m_song_header(void);
void grab_s3m_parapointers(FILE* file);
void convert_song_header(void);
void convert_song_orders(usize length);
