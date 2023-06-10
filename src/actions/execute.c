#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include "../fwm.h"
#include "../actions.h"
#include "../files.h"
#include "../log.h"

#include "execute.h"

struct fwm_action_arguments {
	char *command;
};

bool fwm_validate_action_execute(const uint8_t **action_ptr, int *length_ptr) {
	const uint8_t *action = *action_ptr;
	int length = *length_ptr;

	if (length < (int)(sizeof (size_t))) return false;

	size_t command_length;
	memcpy(&command_length, action, sizeof (size_t));

	length -= sizeof (size_t);

	if (length < (int)(command_length)) return false;

	action += sizeof (size_t);

	*action_ptr = action;
	*length_ptr = length;

	return true;
}

struct fwm_action *fwm_parse_action_execute(const uint8_t *actions) {
	struct fwm_action *action = calloc(1, sizeof (struct fwm_action));
	if (!action) return NULL;

	struct fwm_action_arguments *arguments = calloc(1, sizeof (struct fwm_action_arguments));
	if (!arguments) {
		free(action);
		return NULL;
	}

	size_t command_length;
	memcpy(&command_length, actions, sizeof command_length);
	actions += sizeof (size_t);

	arguments->command = malloc(command_length + 1);
	if (!arguments->command) {
		free(action);
		free(arguments);
		return NULL;
	}

	arguments->command[command_length] = '\0';
	memcpy(arguments->command, actions, command_length);

	action->run = fwm_run_action_execute;
	action->free = fwm_free_action_execute;
	action->arguments = arguments;

	return action;
}

void fwm_run_action_execute(struct fwm_action_arguments *arguments, xcb_window_t window) {
	(void)(window);

	if (fork() == 0) {
		char *command[4] = { fwm.exec_shell, "-c", arguments->command, NULL };
		FWM_DLOG("Executing \"%s\".\n", command[0]);

		fwm_set_signal_handler(SIG_DFL); /* no real reason to do this */
		fwm_close_files(); /* ideally would set CLOEXEC on those file descriptors */

		execvp(command[0], command);

		fwm_open_log_file(NULL);
		FWM_ELOG("Failed to execute \"%s\".\n", command[0]);
		if (fwm.files.log_file) fclose(fwm.files.log_file);

		exit(EXIT_FAILURE);
	}
}

void fwm_free_action_execute(struct fwm_action *action) {
	free(action->arguments->command);
	free(action->arguments);
	free(action);
}
