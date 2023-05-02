#ifndef FILES_H
#define FILES_H

#include <stdio.h>

void fwm_initialize_files(void);
char *fwm_mkdir_cache(void);
void fwm_open_log_file(char *directory, char *name);

#endif
