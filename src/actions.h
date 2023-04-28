#ifndef ACTIONS_H
#define ACTIONS_H

#include <xcb/xproto.h>

#define FWM_ACTION_CLOSE_FOCUSED 1
#define FWM_ACTION_EXECUTE 2

struct fwm_action {
	void (*run)(void*, xcb_window_t);
	void *args;
	struct fwm_action *next;
};

struct fwm_action_execute_args {
	char *command;
};

void fwm_action_close_focused(void *args, xcb_window_t window);
void fwm_action_execute(void *args, xcb_window_t window);

#endif
