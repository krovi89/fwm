#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <sys/time.h>

#include "fwm.h"
#include "log.h"

static int fwm_log_va_list(FILE *stream,
                           const char *type, const char *type_escape,
                           const char *format_escape, const char *format,
                           va_list arg) {
	int ret = 0;

	time_t current_time = time(NULL);

	if (fwm.log_file) {
		va_list ag;
		va_copy(ag, arg);

		fprintf(fwm.log_file, "%li: %s: ", current_time, type);
		vfprintf(fwm.log_file, format, ag);

		va_end(ag);
	}

	if (stream) {
		fprintf(stream, "%li: %s%s:%s ", current_time, type_escape, type, format_escape);
		ret = vfprintf(stream, format, arg);
	}

	return ret;
}

void fwm_log(enum fwm_log_type type, const char *format, ...) {
	va_list argument_list;
	va_start(argument_list, format);

	switch (type) {
		case FWM_LOG_INFO:
			fwm_log_va_list(stdout, "INFO", /* bold blue */ "\033[34;1m", /* reset */ "\033[0m", format, argument_list);
			break;
		case FWM_LOG_DIAGNOSTIC: {
			FILE *stream;
			if (fwm.show_diagnostics)
				stream = stdout;
			else
				stream = NULL;

			fwm_log_va_list(stream, "DIAGNOSTIC", /* bold magenta */ "\033[31;1m", /* reset */ "\033[0m", format, argument_list);
			break;
		}
		case FWM_LOG_WARNING:
			fwm_log_va_list(stderr, "WARNING", /* bold yellow */ "\033[33;1m", /* reset */ "\033[0m", format, argument_list);
			break;
		case FWM_LOG_ERROR:
			fwm_log_va_list(stderr, "ERROR", /* bold red */ "\033[31;1m", /* reset */ "\033[0m", format, argument_list);
			break;
	}

	va_end(argument_list);
}
