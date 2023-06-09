#ifndef LOG_H
#define LOG_H

#include <stdio.h>

/* These macros rely on a GNU extension (##__VA_ARGS__)
   They also assume that `format` is a string literal */
#define FWM_ELOG(format, ...)                             \
	fwm_log(FWM_LOG_ERROR,                                \
			"%s:%i %s: " format,                          \
		    __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define FWM_DLOG(format, ...)                             \
	fwm_log(FWM_LOG_DIAGNOSTIC,                           \
			"%s:%i %s: " format,                          \
		    __FILE__, __LINE__, __func__, ##__VA_ARGS__)


enum fwm_log_type {
	FWM_LOG_INFO,
	FWM_LOG_DIAGNOSTIC,
	FWM_LOG_WARNING,
	FWM_LOG_ERROR
};

void fwm_log(enum fwm_log_type type, const char *format, ...);

#endif
