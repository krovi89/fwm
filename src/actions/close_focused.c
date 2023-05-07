#include <stdlib.h>
#include <stdint.h>

#include <xcb/xproto.h>

#include "close_focused.h"
#include "../fwm.h"

struct fwm_action *fwm_parse_action_close_focused(const uint8_t *actions) {
	(void)(actions);

	struct fwm_action *action = calloc(1, sizeof (struct fwm_action));
	if (!action) return NULL;

	action->run = fwm_run_action_close_focused;
	action->free = fwm_free_action_close_focused;

	return action;
}

void fwm_run_action_close_focused(void *args, xcb_window_t window) {
	(void)(args);

	xcb_kill_client(fwm.conn, window);
}

void fwm_free_action_close_focused(struct fwm_action *action) {
	free(action);
}

bool fwm_validate_action_close_focused(const uint8_t **action_ptr, int *length_ptr) {
	const uint8_t *action = *action_ptr;
	int length = *length_ptr;

	if (length < 1) return false;

	length--;
	action++;

	*action_ptr = action;
	*length_ptr = length;

	return true;
}
