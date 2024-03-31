#ifndef __PARA_H
#define __PARA_H
#include <stdlib.h>
#include <stdio.h>
#include "ext.h"

u16 calculate_stm_sample_parapointer(usize id, usize sample_size);
usize convert_to_parapointer(usize pointer);
usize convert_from_parapointer(usize parapointer);
#endif
