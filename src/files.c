#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>

#include "fwm.h"
#include "files.h"
#include "log.h"

static bool fwm_is_dir(const char *path);
static bool fwm_mkdir(const char *dir, unsigned int mode, size_t dirlen);

void fwm_initialize_files(void) {
	if (fwm.env.data_dir)
		fwm_mkdir_data(fwm.env.data_dir);
	else
		fwm_mkdir_data(fwm_default_data_path());

	if (fwm.env.log_file)
		fwm_open_log_file(fwm.env.log_file);
	else
		fwm_open_log_file(fwm_default_log_path());
}

static bool fwm_is_dir(const char *path) {
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

const char *fwm_default_data_path(void) {
	static char buf[4096] = { 0 };
	size_t buflen = sizeof buf;

	if (buf[0] != 0)
		return buf;

	int ret;
	if (fwm.env.xdg_data_home && fwm.env.xdg_data_home[0] != '\0') {
		ret = snprintf(buf, buflen, "%s/fwm", fwm.env.xdg_data_home);
	} else if (fwm.env.home && fwm.env.home[0] != '\0') {
		ret = snprintf(buf, buflen, "%s/%s", fwm.env.home, FWM_DATA_DIR);
	} else {
		FWM_ELOG("Could not determine the data directory's path (HOME and XDG_DATA_HOME either unset or empty)");
		return NULL;
	}

	if ((ret > (int)(buflen - 1)) || ret < 0)
		return NULL;

	return buf;
}

const char *fwm_default_log_path(void) {
	if (!fwm.files.data_dir) return NULL;

	static char buf[4096];
	size_t buflen = sizeof buf;

	int ret = snprintf(buf, buflen, "%s/%s", fwm.files.data_dir, FWM_LOG_FILE);
	if ((ret > (int)(buflen - 1)) || ret < 0)
		return NULL;

	return buf;
}

bool fwm_mkdir_data(const char *path) {
	if (!path || path[0] == '\0') return false;

	if (!fwm_mkdir(path, 0700, strlen(path))) {
		FWM_ELOG("Failed to create data directory \"%s\".\n", path);
		return false;
	}

	fwm.files.data_dir = path;

	FWM_DLOG("Created data directory \"%s\".\n", path);
	return true;
}

bool fwm_open_log_file(const char *path) {
	static const char *previous_path = NULL;

	if (path) {
		if (path[0] != '\0')
			previous_path = path;
		else
			return false;
	}

	if (!path) {
		if (previous_path)
			path = previous_path;
		else
			return false;
	}

	FILE *log_file = fopen(path, "a");
	if (!log_file) {
		FWM_ELOG("Failed to open log file \"%s\": %s\n", path, strerror(errno));
		return false;
	}

	if (fwm.files.log_file)
		fclose(fwm.files.log_file);

	fwm.files.log_file = log_file;

	FWM_DLOG("Opened log file \"%s\".\n", path);
	return true;
}
