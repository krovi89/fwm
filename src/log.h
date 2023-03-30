#ifndef LOG_H
#define LOG_H

int fwm_log_error(char *format, ...);
void fwm_log_error_exit(int ret, char *format, ...);
int fwm_log_warning(char *format, ...);
int fwm_log_info(char *format, ...);

#endif
