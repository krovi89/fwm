#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "fwm.h"
#include "log.h"

static int fwm_log(FILE *stream, const char *type, const char *type_escape, const char *message_escape, const char *format, va_list argument_list) {
	fprintf(stream, "%s%s:%s ", type_escape, type, message_escape);
	int ret = vfprintf(stream, format, argument_list);

	return ret;
}

int fwm_log_error(const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	int ret = fwm_log(stderr, "ERROR", /* bold red */ "\033[31;1m", /* reset */ "\033[0m", format, argument_list);
	va_end(argument_list);

	return ret;
}

void fwm_log_error_exit(int ret, const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	fwm_log(stderr, "ERROR", /* bold red */ "\033[31;1m", /* reset */ "\033[0m", format, argument_list);
	va_end(argument_list);

	fwm_exit(ret);
}

int fwm_log_warning(const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	int ret = fwm_log(stderr, "WARNING", /* bold yellow */ "\033[33;1m", /* reset */ "\033[0m", format, argument_list);
	va_end(argument_list);

	return ret;
}

int fwm_log_info(const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);
	int ret = fwm_log(stdout, "INFO", /* bold blue */ "\033[34;1m", /* reset */ "\033[0m", format, argument_list);
	va_end(argument_list);

	return ret;
}
