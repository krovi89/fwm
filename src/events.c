#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "events.h"
#include "fwm.h"
#include "actions.h"
#include "keybinds.h"

void fwm_handle_event(xcb_generic_event_t *event) {
	switch (event->response_type) {
		case XCB_MAP_REQUEST:
			fwm_event_map_request((xcb_map_request_event_t*)(event));
			break;
		case XCB_CONFIGURE_REQUEST:
			fwm_event_configure_request((xcb_configure_request_event_t*)(event));
			break;
		case XCB_KEY_PRESS:
			fwm_event_key_press((xcb_key_press_event_t*)(event));
			break;
		default:
			break;
	}
}

void fwm_event_map_request(xcb_map_request_event_t *event) {
	xcb_map_window(fwm.conn, event->window);
}

void fwm_event_configure_request(xcb_configure_request_event_t *event) {
	int i = 0;
	int32_t value_list[7] = { 0 };

	if (event->value_mask & XCB_CONFIG_WINDOW_X)
		value_list[i++] = event->x;
	if (event->value_mask & XCB_CONFIG_WINDOW_Y)
		value_list[i++] = event->y;
	if (event->value_mask & XCB_CONFIG_WINDOW_WIDTH)
		value_list[i++] = event->width;
	if (event->value_mask & XCB_CONFIG_WINDOW_HEIGHT)
		value_list[i++] = event->height;
	if (event->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH)
		value_list[i++] = event->border_width;
	if (event->value_mask & XCB_CONFIG_WINDOW_SIBLING)
		value_list[i++] = event->sibling;
	if (event->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)
		value_list[i++] = event->stack_mode;

	xcb_configure_window(fwm.conn, event->window, event->value_mask, value_list);
}

void fwm_event_key_press(xcb_key_press_event_t *event) {
	struct fwm_keybind *keybind = fwm.current_position;
	while (keybind) {
		if (event->state == keybind->keymask && event->detail == keybind->keycode) {
			if (!keybind->child) {
				struct fwm_action *action;
				for (action = keybind->actions; action; action = action->next) {
					action->run(action->arguments, event->child);
				}

				fwm_set_keybinds_position(fwm.keybinds);
			} else {
				fwm_set_keybinds_position(keybind->child);
			}

			break;
		}

		keybind = keybind->next;
	}
}
