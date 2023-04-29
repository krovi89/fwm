#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <xcb/xproto.h>

#include "actions.h"
#include "fwm.h"
#include "log.h"

void fwm_action_execute(void *args, xcb_window_t window) {
	(void)(window);
	struct fwm_action_execute_args *execute_args = args;

	if (fork() == 0) {
		fwm_close_fds();

		char *command[4] = { fwm.exec_shell, "-c", execute_args->command, NULL };
		execvp(command[0], command);

		fwm_log_error("Failed to spawn \"%s\"\n", fwm.exec_shell);
		exit(EXIT_FAILURE);
	}
}

void fwm_action_close_focused(void *args, xcb_window_t window) {
	(void)(args);
	xcb_kill_client(fwm.conn, window);
}
