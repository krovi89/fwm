#include <xcb/xproto.h>

#include "actions.h"
#include "fwm.h"

void fwm_action_close_focused(xcb_key_press_event_t *event) {
	xcb_kill_client(fwm.conn, event->child);
}
