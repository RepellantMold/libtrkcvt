#include <string.h>

#include "envcheck.h"
#include "ext.h"
#include "log.h"
#include "struct.h"

#include "valid.h"

#include "fmt/s3m.h"
#include "fmt/stm.h"
#include "fmt/stx.h"

int check_valid_s3m(FILE* S3Mfile) {
  char scrm[4] = {0};

  if (!S3Mfile)
    return FOC_MALFORMED_FILE;

  (void)!fseek(S3Mfile, 44, SEEK_SET);

  if (fread(scrm, sizeof(char), 4, S3Mfile) != 4) {
    print_error("Failed to read S3M header!");
    return FOC_MALFORMED_FILE;
  };

  if (memcmp(scrm, "SCRM", 4)) {
    print_error("This is not an S3M file!");
    return FOC_NOT_S3M_FILE;
  }

  rewind(S3Mfile);
  return FOC_SUCCESS;
}
