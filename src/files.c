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
	fwm.cache_dir = fwm_mkdir_cache();
	fwm_open_log_file(fwm.cache_dir, "fwm.log");
}

uint8_t fwm_is_directory(const char *directory) {
	struct stat stat_buf;
	if (stat(directory, &stat_buf) == -1) return false;
	if (S_ISDIR(stat_buf.st_mode)) return true;
	return false;
}

#define FUNC_NAME "fwm_mkdir"
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
				fwm_log(FWM_LOG_ERROR, "%s: Failed to create directory \"%s\": %s\n", FUNC_NAME, copy, strerror(errno));
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
#undef FUNC_NAME

#define FUNC_NAME "fwm_mkdir_cache"
char *fwm_mkdir_cache(void) {
	char *home = getenv("XDG_CACHE_HOME");
	static char path[4096];

	int ret;
	if (home) {
		if (home[0] == '\0') return NULL;
		ret = snprintf(path, sizeof path, "%s/fwm", home);
	} else {
		home = getenv("HOME");
		if (!home || home[0] == '\0') return NULL;
		ret = snprintf(path, sizeof path, "%s/.cache/fwm", home);
	}

	if ((ret > (int)(sizeof path - 1)) || ret < 0)
		return NULL;

	if (!fwm_mkdir(path, 0700, ret)) return NULL;

	fwm_log(FWM_LOG_DIAGNOSTIC, "%s: Created cache directory \"%s\".\n", FUNC_NAME, path);
	return path;
}
#undef FUNC_NAME

#define FUNC_NAME "fwm_open_log_file"
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
		fwm_log(FWM_LOG_ERROR, "%s: Failed to open file \"%s\": %s\n", FUNC_NAME, path, strerror(errno));
		return;
	}

	fwm.log_file = log_file;
	fwm_log(FWM_LOG_DIAGNOSTIC, "%s: Opened log file \"%s\".\n", FUNC_NAME, path);
}
#undef FUNC_NAME
