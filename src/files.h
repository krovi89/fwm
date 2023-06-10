#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdbool.h>

#define FWM_DATA_DIR ".local/share/fwm"
#define FWM_LOG_FILE "fwm.log"

struct fwm_files {
	char *data_dir;

	FILE *log_file;
};

void fwm_initialize_files(void);

bool fwm_build_data_path(char *buf, size_t buflen);
bool fwm_mkdir_data(char *path);

bool fwm_build_log_path(char *buf, size_t buflen);
bool fwm_open_log_file(char *path);

#endif
