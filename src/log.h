#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>

int fwm_log(FILE *stream, char *type, char *type_escape, char *message_escape, char *format, va_list argument_list);
int fwm_log_error(char *format, ...);

#endif
