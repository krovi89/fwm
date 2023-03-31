#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "fwm.h"
#include "events.h"

void fwm_initialize_event_handlers(void) {
	fwm.event_handlers.map_request = fwm_event_map_request;
}

void fwm_handle_event(xcb_generic_event_t *event) {
	switch (event->response_type) {
		case XCB_MAP_REQUEST:
			fwm.event_handlers.map_request((xcb_map_request_event_t*) event);
		default:
			break;
	}
}

void fwm_event_map_request(xcb_map_request_event_t *event) {
	xcb_map_window(fwm.conn, event->window);
}
