#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <signal.h>

#include <xcb/xproto.h>

#include "actions.h"
#include "fwm.h"
#include "files.h"
#include "log.h"

#define FUNC_NAME "fwm_action_execute"
void fwm_action_execute(void *args, xcb_window_t window) {
	(void)(window);

	struct fwm_action_execute_args *execute_args = args;

	if (fork() == 0) {
		fwm_set_signal_handler(SIG_DFL); // no actual reason to do this
		fwm_close_files();

		char *command[4] = { fwm.exec_shell, "-c", execute_args->command, NULL };
		execvp(command[0], command);

		fwm_open_log_file(NULL, NULL);
		fwm_log(FWM_LOG_DIAGNOSTIC, "%s: Failed to spawn \"%s\"\n", FUNC_NAME, fwm.exec_shell);
		if (fwm.log_file) fclose(fwm.log_file);

		exit(EXIT_FAILURE);
	}
}
#undef FUNC_NAME

void fwm_action_close_focused(void *args, xcb_window_t window) {
	(void)(args);
	xcb_kill_client(fwm.conn, window);
}
