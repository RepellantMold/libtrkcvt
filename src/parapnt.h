#ifndef __PARA_H
#define __PARA_H
#include <stdio.h>
#include <stdlib.h>

#include "ext.h"

u16 calculate_stm_sample_parapointer(void);
usize convert_to_parapointer(usize pointer);
usize convert_from_parapointer(usize parapointer);
#endif
