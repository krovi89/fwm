#ifndef ACTIONS_H
#define ACTIONS_H

#include <xcb/xproto.h>

#define FWM_ACTION_CLOSE_FOCUSED 1

struct fwm_action {
	void (*run)(xcb_key_press_event_t*);
	struct fwm_action *next;
};

void fwm_action_close_focused(xcb_key_press_event_t *event);

#endif
