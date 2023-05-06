#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

#include <xcb/xproto.h>

#include "actions.h"
#include "fwm.h"
#include "files.h"
#include "log.h"

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

void fwm_run_action_close_focused(void *args, xcb_window_t window) {
	(void)(args);
	xcb_kill_client(fwm.conn, window);
}

void fwm_free_action_close_focused(struct fwm_action *action) {
	free(action);
}
