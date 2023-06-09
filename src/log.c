#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/time.h>
#include <time.h>

#include "fwm.h"
#include "log.h"

static int fwm_log_va_list(FILE *stream,
                           const char *type_escape, const char *type,
                           const char *format_escape, const char *format,
                           va_list arg) {
	int ret = 0;

	if (fwm.files.log_file) {
		va_list ag;
		va_copy(ag, arg);

		struct tm *tm = localtime(&(time_t){ time(NULL) });
		fprintf(fwm.files.log_file, "[%i/%02i/%02i %02i:%02i:%02i] %s: ", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		                                                                  tm->tm_hour, tm->tm_min, tm->tm_sec,
		                                                                  type);
		vfprintf(fwm.files.log_file, format, ag);

		va_end(ag);
		fflush(fwm.files.log_file);
	}

	if (stream) {
		fprintf(stream, "%s%s:%s ", type_escape, type, format_escape);
		ret = vfprintf(stream, format, arg);
		fflush(stream);
	}

	return ret;
}

void fwm_log(enum fwm_log_type type, const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);

	switch (type) {
		case FWM_LOG_INFO:
			fwm_log_va_list(stdout, /* bold blue */ "\033[34;1m", "INFO", /* reset */ "\033[0m", format, argument_list);
			break;
		case FWM_LOG_DIAGNOSTIC: {
			FILE *stream;
			if (fwm.show_diagnostics)
				stream = stdout;
			else
				stream = NULL;

			fwm_log_va_list(stream, /* bold magenta */ "\033[35;1m", "DIAGNOSTIC", /* reset */ "\033[0m", format, argument_list);
			break;
		}
		case FWM_LOG_WARNING:
			fwm_log_va_list(stderr, /* bold yellow */ "\033[33;1m", "WARNING", /* reset */ "\033[0m", format, argument_list);
			break;
		case FWM_LOG_ERROR:
			fwm_log_va_list(stderr, /* bold red */ "\033[31;1m", "ERROR", /* reset */ "\033[0m", format, argument_list);
			break;
	}

	va_end(argument_list);
}
