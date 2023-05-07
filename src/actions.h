#ifndef ACTIONS_H
#define ACTIONS_H

#include <xcb/xproto.h>

#define FWM_ACTION_CLOSE_FOCUSED 1
#define FWM_ACTION_EXECUTE 2
#define FWM_MAX_ACTION 2

struct fwm_action {
	void (*run)(void*, xcb_window_t);
	void (*free)(struct fwm_action*);
	void *args;
	struct fwm_action *next;
};

struct fwm_action_execute_args {
	char *command;
};

void fwm_initialize_actions(void);
void fwm_initialize_action_validators(void);
void fwm_free_actions(struct fwm_action *actions);

#endif
