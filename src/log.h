#ifndef LOG_H
#define LOG_H

int fwm_log_error(const char *format, ...);
void fwm_log_error_exit(int ret, const char *format, ...);
int fwm_log_warning(const char *format, ...);
int fwm_log_info(const char *format, ...);

#endif
