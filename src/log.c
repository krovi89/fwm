#include <stdio.h>
#include <stdarg.h>

#include "log.h"

int fwm_log(FILE *stream, char *type, char *type_escape, char *message_escape, char *format, va_list argument_list) {
	fprintf(stream, "%s%s:%s ", type_escape, type, message_escape);
	int ret = vfprintf(stream, format, argument_list);
	fprintf(stream, "\n");

	return ret;
}

int fwm_log_error(char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	int ret = fwm_log(stderr, "ERROR", /* bold red */ "\033[31;1m", /* reset */ "\033[0m", format, argument_list);
	va_end(argument_list);

	return ret;
}
