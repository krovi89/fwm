#ifndef LOG_H
#define LOG_H

#include <stdio.h>

enum fwm_log_type {
	FWM_LOG_INFO,
	FWM_LOG_DIAGNOSTIC,
	FWM_LOG_WARNING,
	FWM_LOG_ERROR
};

void fwm_log(enum fwm_log_type type, const char *format, ...);

#endif
