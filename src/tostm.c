#include "conv.h"

// Arrays
u8 stm_song_header[48] = {
    // song title (ASCIIZ)
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
    '\0',

    // tracker name
    '!', 'S', 'c', 'r', 'v', 'r', 't', '!',

    // DOS EOF
    0x1A,

    // file type (in this case, a module.)
    2,

    // major version, minor version
    2, 21,

    // tempo (default)
    0x60,

    // number of patterns
    0,

    // global volume
    64,

    // reserved (which I, RM, turned into a magic string, you're welcome!)
    'S', 'c', 'r', 'e', 'a', 'm', 'v', 'e', 'r', 't', 'e', 'r', '\0'};

u8 stm_sample_header[32] = {
    // filename (ASCIIZ)
    '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',

    // reserved
    0,

    // instrument disk
    0,

    /* parapointer to the PCM data
    the official documentation is extremely misleading here since it calls this area reserved!
    "(used as internal segment while playing)" */
    0, 0,

    // length in bytes
    0, 0,

    // loop start
    0, 0,

    // loop end (0xFFFF means no loop)
    0xFF, 0xFF,

    // volume
    64,

    // reserved
    0,

    // speed for C2/Mid-C (calculated as Hz, with a default of 8448 or 8192 depending on version)
    0x00, 0x21,

    // reserved
    0, 0, 0, 0,

    /* reserved, contrary to what the official documentation says...
       "internal segment address/(in modules:)length in paragraphs" */
    0, 0};

static u8 sample_data[USHRT_MAX] = {0};
u8 stm_order_list[STM_ORDER_LIST_SIZE] = {STM_ORDER_END};
u8 stm_pattern[64][4][4] = {{{0xFF, 0x01, 0x80, 0x00}}};

u16 stm_pcm_pointers[STM_MAXSMP] = {0};

static int handle_pcm_s3mtostm(FOC_Context* context, usize sample_count);
static void handle_pcm_parapointer_s3mtostm(FOC_Context* context, usize i);
static void handle_sample_headers_s3mtostm(FOC_Context* context, usize sample_count);
static void handle_patterns_s3mtostm(FOC_Context* context, usize pattern_count);

int convert_s3m_to_stm(FOC_Context* context) {
  FILE *S3Mfile = context->infile, *STMfile = context->outfile;
  const bool verbose = context->verbose_mode;

  if (!S3Mfile || !STMfile)
    return FOC_OPEN_FAILURE;
  if (ferror(S3Mfile) || ferror(STMfile))
    return FOC_MALFORMED_FILE;
  if (check_valid_s3m(S3Mfile))
    return FOC_NOT_S3M_FILE;

  (void)!fread(s3m_song_header, sizeof(u8), sizeof(s3m_song_header), S3Mfile);
  s3m_cwtv = s3m_song_header[41] << 8 | s3m_song_header[40];
  original_order_count = s3m_song_header[32];
  sample_count = s3m_song_header[34];
  if (sample_count > STM_MAXSMP)
    print_warning("Sample count exceeds 31 (%u > 31), only using 31.", sample_count);
  pattern_count = s3m_song_header[36];
  if (pattern_count > STM_MAXPAT)
    print_warning("Pattern count exceeds 63 (%u > 63), only converting 63.", pattern_count);
  if (verbose)
    show_s3m_song_header();

  check_s3m_channels();

  convert_song_header_s3mtostm();
  fwrite(stm_song_header, sizeof(u8), sizeof(stm_song_header), STMfile);

  grab_s3m_orders(S3Mfile);
  grab_s3m_parapointers(S3Mfile);

  handle_sample_headers_s3mtostm(context, sample_count);

  convert_song_orders_s3mtostm(order_count);
  fwrite(stm_order_list, sizeof(u8), sizeof(stm_order_list), STMfile);

  handle_patterns_s3mtostm(context, pattern_count);

  if (handle_pcm_s3mtostm(context, sample_count))
    return FOC_SAMPLE_FAIL;

  puts("Conversion done successfully!");
  return FOC_SUCCESS;
}

static void handle_sample_headers_s3mtostm(FOC_Context* context, usize sample_count) {
  FILE *S3Mfile = context->infile, *STMfile = context->outfile;
  const bool verbose = context->verbose_mode;
  usize i = 0;

  for (; i < STM_MAXSMP; i++) {
    if (i >= sample_count) {
      generate_blank_stm_instrument();
      goto skiptowritingsampleheader;
    }

    if (verbose)
      printf("Sample %zu:\n", i);

    grab_sample_data(S3Mfile, s3m_inst_pointers[i]);
    s3m_pcm_pointers[i] = grab_s3m_pcm_pointer();
    s3m_pcm_lens[i] = grab_s3m_pcm_len();

    if (verbose)
      show_s3m_inst_header();

    convert_s3m_intstrument_header_s3mtostm();

  skiptowritingsampleheader:
    fwrite(stm_sample_header, sizeof(u8), sizeof(stm_sample_header), STMfile);
  }
}

static void handle_patterns_s3mtostm(FOC_Context* context, usize pattern_count) {
  FILE *S3Mfile = context->infile, *STMfile = context->outfile;
  usize i = 0;

  for (i = 0; i < STM_MAXPAT; i++) {
    if (i >= pattern_count)
      break;
    printf("Converting pattern %zu...\n", i);
    parse_s3m_pattern(S3Mfile, s3m_pat_pointers[i]);
    convert_s3m_pattern_to_stm();
    fwrite(stm_pattern, sizeof(u8), sizeof(stm_pattern), STMfile);
    printf("Pattern %zu written.\n", i);
  }
}

static int handle_pcm_s3mtostm(FOC_Context* context, usize sample_count) {
  FILE *S3Mfile = context->infile, *STMfile = context->outfile;
  usize i = 0;
  Sample_Context sc;
  usize sample_len = 0, padding_len = 0;

  for (; i < STM_MAXSMP; i++) {
    if (i >= sample_count)
      break;

    sample_len = s3m_pcm_lens[i];

    if (!sample_len)
      continue;

    padding_len = (u16)calculate_sample_padding(sample_len);

    sc.length = sample_len;
    sc.pcm = sample_data;

    printf("Converting sample %zu...\n", i);

    if (dump_sample_data(S3Mfile, s3m_pcm_pointers[i], &sc))
      return FOC_SAMPLE_FAIL;

    convert_unsigned_to_signed(&sc);

    if (!padding_len)
      goto dontaddpadding;

    sample_len += padding_len;

  dontaddpadding:
    handle_pcm_parapointer_s3mtostm(context, i);

    (void)!fwrite(sample_data, sizeof(u8), sample_len, STMfile);
    (void)!printf("Sample %zu written.\n", i);
  }

  return FOC_SUCCESS;
}

static void handle_pcm_parapointer_s3mtostm(FOC_Context* context, usize i) {
  const usize saved_pos = (usize)ftell(context->outfile),
              header_pos = sizeof(stm_song_header) + ((sizeof(stm_sample_header) * (i + 1)) - 18);

  stm_pcm_pointers[i] = calculate_stm_sample_parapointer();

  (void)!fseek(context->outfile, (long)header_pos, SEEK_SET);

  (void)!fwrite(&stm_pcm_pointers[i], sizeof(u8), 2, context->outfile);

  (void)!fseek(context->outfile, (long)saved_pos, SEEK_SET);
}
