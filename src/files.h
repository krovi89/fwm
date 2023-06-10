#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdbool.h>

#define FWM_DATA_DIR ".local/share/fwm"
#define FWM_LOG_FILE "fwm.log"

struct fwm_files {
	const char *data_dir;

	FILE *log_file;
};

void fwm_initialize_files(void);

const char *fwm_default_data_path(void);
bool fwm_mkdir_data(const char *path);

const char *fwm_default_log_path(void);
bool fwm_open_log_file(const char *path);

#endif
