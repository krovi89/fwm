#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <sys/stat.h>

#include "files.h"
#include "fwm.h"

void fwm_initialize_files(void) {
	fwm.cache_dir = fwm_mkdir_cache();

	fwm_open_log_file(fwm.cache_dir, "fwm.log");
}

bool fwm_mkdir(const char *directory, unsigned int mode, int length) {
	if (!directory || directory[0] != '/') return false;

	struct stat stat_buf;
	if (stat(directory, &stat_buf) == 0) return true;

	char *buffer = malloc(length + 1);
	if (!buffer) return false;

	strcpy(buffer, directory);
	char *tmp = buffer;

	while ((tmp = strchr(tmp + 1, '/'))) {
		*tmp = '\0';

		if (stat(buffer, &stat_buf) == -1)
			if (mkdir(buffer, mode) == -1) {
				free(buffer);
				return false;
			}

		*tmp = '/';
	}

	if (stat(buffer, &stat_buf) == -1)
		if (mkdir(buffer, mode) == -1) {
			free(buffer);
			return false;
		}

	free(buffer);
	return true;
}

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
		return;
	}

	fwm.log_file = log_file;
}
