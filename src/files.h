#ifndef FILES_H
#define FILES_H

#include <stdbool.h>

void fwm_initialize_files(void);
bool fwm_mkdir(const char *directory, unsigned int mode, int length);
char *fwm_mkdir_cache(void);
void fwm_open_log_file(const char *directory, char *name);

#endif
