#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <signal.h>

#include "execute.h"
#include "../fwm.h"
#include "../actions.h"
#include "../files.h"
#include "../log.h"

struct fwm_action *fwm_parse_action_execute(const uint8_t *actions) {
	struct fwm_action *action = calloc(1, sizeof (struct fwm_action));
	if (!action) return NULL;

	struct fwm_action_execute_args *args = calloc(1, sizeof (struct fwm_action_execute_args));
	if (!args) {
		free(action);
		return NULL;
	}

	size_t command_length;
	memcpy(&command_length, actions, sizeof command_length);
	actions += sizeof (size_t);

	args->command = malloc(command_length + 1);
	if (!args->command) {
		free(action);
		free(args);
		return NULL;
	}

	args->command[command_length] = '\0';
	memcpy(args->command, actions, command_length);

	action->run = fwm_run_action_execute;
	action->free = fwm_free_action_execute;
	action->args = args;

	return action;
}

void fwm_run_action_execute(void *args, xcb_window_t window) {
	(void)(window);

	struct fwm_action_execute_args *execute_args = args;

	if (fork() == 0) {
		char *command[4] = { fwm.exec_shell, "-c", execute_args->command, NULL };
		fwm_log(FWM_LOG_DIAGNOSTIC, "Executing \"%s\".\n", command[0]);

		fwm_set_signal_handler(SIG_DFL); // no actual reason to do this
		fwm_close_files();

		execvp(command[0], command);

		fwm_open_log_file(NULL, NULL);
		fwm_log(FWM_LOG_ERROR, "Failed to execute \"%s\".\n", command[0]);
		if (fwm.log_file) fclose(fwm.log_file);

		exit(EXIT_FAILURE);
	}
}

void fwm_free_action_execute(struct fwm_action *action) {
	struct fwm_action_execute_args *execute_args = action->args;
	free(execute_args->command);
	free(execute_args);
	free(action);
}

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
