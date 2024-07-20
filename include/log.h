#ifndef __LOG_H
#define __LOG_H

#ifdef ENABLE_LOG
/* explicitly define your own functions! */
extern print_diagnostic(char* msg, ...);
extern print_error(char* msg, ...);
extern print_warning(char* msg, ...);
#else
#define print_diagnostic(msg, ...)
#define print_error(msg, ...)
#define print_warning(msg, ...)
#endif

#endif
