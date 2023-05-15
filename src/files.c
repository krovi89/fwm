#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

#include "files.h"
#include "fwm.h"
#include "log.h"

void fwm_initialize_files(void) {
	fwm.data_dir = fwm_mkdir_data();
	fwm_open_log_file(fwm.data_dir, "fwm.log");
}

uint8_t fwm_is_directory(const char *directory) {
	struct stat stat_buf;
	if (stat(directory, &stat_buf) == -1) return false;
	if (S_ISDIR(stat_buf.st_mode)) return true;
	return false;
}

bool fwm_mkdir(const char *directory, unsigned int mode, int length) {
	if (!directory || directory[0] != '/') return false;
	if (fwm_is_directory(directory)) return true;

	char *copy = malloc(length + 1);
	if (!copy) return false;

	strcpy(copy, directory);
	char *sep = copy;
	char *prev_sep = sep;

	while (sep) {
		sep = strchr(sep + 1, '/');

		if (sep) {
			if (sep == prev_sep + 1 ||
			    sep == copy + length - 1) continue;

			*sep = '\0';
		}

		if (!fwm_is_directory(copy))
			if (mkdir(copy, mode) == -1) {
				fwm_log(FWM_LOG_ERROR, "Failed to create directory \"%s\": %s\n", copy, strerror(errno));
				free(copy);
				return false;
			}

		if (sep) {
			*sep = '/';
			prev_sep = sep;
		}
	}

	free(copy);
	return true;
}

char *fwm_mkdir_data(void) {
	char *env = getenv("XDG_DATA_HOME");
	static char path[4096];

	int ret;
	if (env) {
		if (env[0] == '\0') return NULL;
		ret = snprintf(path, sizeof path, "%s/fwm", env);
	} else {
		env = getenv("HOME");
		if (!env || env[0] == '\0') return NULL;
		ret = snprintf(path, sizeof path, "%s/.local/share/fwm", env);
	}

	if ((ret > (int)(sizeof path - 1)) || ret < 0)
		return NULL;

	if (!fwm_mkdir(path, 0700, ret)) return NULL;

	fwm_log(FWM_LOG_DIAGNOSTIC, "Created data directory \"%s\".\n", path);
	return path;
}

void fwm_open_log_file(const char *directory, char *file_name) {
	static char path[4096];

	if (directory && file_name) {
		int ret = snprintf(path, sizeof path, "%s/%s", directory, file_name);
		if ((ret > (int)(sizeof path - 1)) || ret < 0)
			return;

		if (fwm.log_file) {
			fclose(fwm.log_file);
			fwm.log_file = NULL;
		}
	}

	if (path[0] == '\0') return;

	FILE *log_file = fopen(path, "a");
	if (!log_file) {
		fwm_log(FWM_LOG_ERROR, "Failed to open file \"%s\": %s\n", path, strerror(errno));
		return;
	}

	fwm.log_file = log_file;
	fwm_log(FWM_LOG_DIAGNOSTIC, "Opened log file \"%s\".\n", path);
}
