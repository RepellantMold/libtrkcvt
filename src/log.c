#include <stdio.h>
#include <stdarg.h>

#include "envcheck.h"
#include "ext.h"
#include "main.h"

void print_diagnostic(const char* format, ...) {
  if (!main_context.verbose_mode)
    return;

  va_list ap;
  char buffer[192];

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);

  printf("VERBOSE: %s\n", buffer);
}

void print_warning(const char* format, ...) {
  va_list ap;
  char buffer[192];

  va_start(ap, format);
  vsnprintf(buffer, sizeof(buffer), format, ap);
  va_end(ap);

  printf("WARNING: %s\n", buffer);
}

void print_error(const char* format, ...) {
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
}