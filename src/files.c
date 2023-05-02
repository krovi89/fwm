#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <sys/stat.h>

#include "files.h"
#include "fwm.h"

void fwm_initialize_files(void) {
	fwm.cache_dir = fwm_create_cache();

	fwm_initialize_log_file(fwm.cache_dir);
}

char *fwm_create_cache(void) {
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

	struct stat stat_buf;
	if (stat(path, &stat_buf) == 0) return path;
	// TODO: Create the directory's parents
	if (mkdir(path, 0700) == -1) return NULL;

	return path;
}

void fwm_initialize_log_file(char *directory) {
	if (!directory) return;

	static char path[4096];

	time_t current_time = time(NULL);
	int ret = snprintf(path, sizeof path, "%s/fwm-%li.log", directory, current_time);
	if ((ret > (int)(sizeof path - 1)) || ret < 0)
		return;

	FILE *log_file = fopen(path, "w");
	if (!log_file) {
		return;
	}

	fwm.log_file = log_file;
}
