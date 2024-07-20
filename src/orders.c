#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "envcheck.h"
#include "ext.h"

#include "file.h"
#include "log.h"
#include "struct.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"
#include "fmt/stx.h"

void grab_s3m_orders(FILE* file) {
  register usize i = 0, count = 0;

  if (!file || feof(file) || ferror(file))
    return;

  fseek(file, S3M_ORDERPOS, SEEK_SET);

  (void)!fread(s3m_order_array, sizeof(u8), main_context.stats.original_order_count, file);

  /* see section "2.6 Load Order Data" from "FireLight S3M Player Tutorial.txt" */
  for (count = 0; count < main_context.stats.original_order_count; ++count) {
    if (s3m_order_array[count] < S3M_ORDER_MARKER) {
      s3m_order_array[i] = s3m_order_array[count];
      if (s3m_order_array[count] > main_context.stats.pattern_count)
        main_context.stats.pattern_count = s3m_order_array[count];
      print_diagnostic("Order %zu -> %zu", count, i);
      i++;
    }
  }
  main_context.stats.order_count = (u8)i;
}

void convert_song_orders_s3mtostm(usize length) {
  register usize i = 0;

  memset(stm_order_list, STM_ORDER_END, STM_ORDER_LIST_SIZE);

  do {
    if (i >= length)
      break;

    stm_order_list[i] = (s3m_order_array[i] > STM_MAXPAT) ? STM_ORDER_END : s3m_order_array[i];
  } while (++i < STM_ORDER_LIST_SIZE);
}

void convert_song_orders_s3mtostx(usize length, u8* order_list) {
  register usize i = 0;

  if (!order_list)
    return;

  do {
    /* I have no idea why STX specifically has its order list like this... */
    order_list[i * STX_ORDERMULTIPLIER] = s3m_order_array[i];
  } while (++i < length);
}