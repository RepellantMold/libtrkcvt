enum FOC_ConversionMode { FOC_S3MTOSTM = 0x00, FOC_S3MTOSTX = 0x01 };

typedef struct {
  FILE* infile;
  FILE* outfile;
  enum FOC_ConversionMode conversion_type;

  struct {
    bool verbose_mode;
    bool sanitize_sample_names;
    bool handle_effect_memory;
  } flags;

} internal_state_t;