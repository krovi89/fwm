#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

#include "fwm.h"
#include "files.h"
#include "log.h"

static uint8_t fwm_is_dir(const char *path);
static bool fwm_mkdir(const char *dir, unsigned int mode, size_t dirlen);

void fwm_initialize_files(void) {
	static char data_dir_buf[4096];
	static char log_file_path_buf[4096];

	if (fwm.env.data_dir)
		fwm_mkdir_data(fwm.env.data_dir);
	else
		if (fwm_build_data_path(data_dir_buf, sizeof data_dir_buf))
			fwm_mkdir_data(data_dir_buf);

	if (fwm.env.log_file_path)
		fwm_open_log_file(fwm.env.log_file_path);
	else
		if (fwm_build_log_path(log_file_path_buf, sizeof log_file_path_buf))
			fwm_open_log_file(log_file_path_buf);
}

static uint8_t fwm_is_dir(const char *path) {
	struct stat statbuf;
	if (stat(path, &statbuf) == -1) return false;
	if (S_ISDIR(statbuf.st_mode)) return true;
	return false;
}

/* Recursively creates directory, as in mkdir -p */
static bool fwm_mkdir(const char *dir, unsigned int mode, size_t dirlen) {
	if (!dir || dir[0] == '\0') return false;
	if (fwm_is_dir(dir)) return true;

	char *copy = malloc(dirlen + 1);
	if (!copy) return false;

	strncpy(copy, dir, dirlen + 1);
	char *sep = copy;
	char *prev_sep = sep;

	while (sep) {
		sep = strchr(sep + 1, '/');

		if (sep) {
			if (sep == prev_sep + 1 ||
				/* last character before null terminator */
				sep == copy + dirlen - 2) continue;

			*sep = '\0';
		}

		if (!fwm_is_dir(copy))
			if (mkdir(copy, mode) == -1) {
				FWM_ELOG("Failed to create directory \"%s\": %s\n", copy, strerror(errno));
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

bool fwm_build_data_path(char *buf, size_t buflen) {
	if (!buf) return false;

	int ret;
	if (fwm.env.xdg_data_home && fwm.env.xdg_data_home[0] != '\0') {
		ret = snprintf(buf, buflen, "%s/fwm", fwm.env.xdg_data_home);
	} else if (fwm.env.home && fwm.env.home[0] != '\0') {
		ret = snprintf(buf, buflen, "%s/%s", fwm.env.home, FWM_DATA_DIR);
	} else {
		FWM_ELOG("Could not determine the data directory's path (HOME and XDG_DATA_HOME either unset or empty)");
		return false;
	}

	if ((ret > (int)(buflen - 1)) || ret < 0)
		return false;

	return true;
}

bool fwm_mkdir_data(char *path) {
	if (!path || path[0] == '\0') return false;

	if (!fwm_mkdir(path, 0700, strlen(path)))
		return false;

	fwm.files.data_dir = path;

	FWM_DLOG("Created data directory \"%s\".\n", path);
	return true;
}

bool fwm_build_log_path(char *buf, size_t buflen) {
	if (!buf) return false;
	if (!fwm.files.data_dir) return false;

	int ret = snprintf(buf, buflen, "%s/%s", fwm.files.data_dir, FWM_LOG_FILE);
	if ((ret > (int)(buflen - 1)) || ret < 0)
		return false;

	return true;
}

bool fwm_open_log_file(char *path) {
	char *previous_path = NULL;

	if (path) {
		if (path[0] != '\0')
			previous_path = path;
		else
			return false;
	}

	if (!path && previous_path)
		path = previous_path;

	FILE *log_file = fopen(path, "a");
	if (!log_file) {
		FWM_ELOG("Failed to open file \"%s\": %s\n", path, strerror(errno));
		return false;
	}

	if (fwm.files.log_file)
		fclose(fwm.files.log_file);

	fwm.files.log_file = log_file;

	FWM_DLOG("Opened log file \"%s\".\n", path);
	return true;
}
