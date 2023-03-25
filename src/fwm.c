#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>

#include "fwm.h"
#include "log.h"

struct fwm fwm;

int main(void) {
	fwm_initialize();
	fwm_register_events();

	fwm_exit(0);
}

void fwm_initialize(void) {
	int preferred_screen; fwm.conn = xcb_connect(NULL, &preferred_screen);

	int error_code = xcb_connection_has_error(fwm.conn);
	if (error_code) {
		char *error_strings[XCB_CONN_CLOSED_INVALID_SCREEN + 1] = {
		                   [XCB_CONN_ERROR]                   = "XCB_CONN_ERROR",
		                   [XCB_CONN_CLOSED_EXT_NOTSUPPORTED] = "XCB_CONN_CLOSED_EXT_NOTSUPPORTED",
		                   [XCB_CONN_CLOSED_MEM_INSUFFICIENT] = "XCB_CONN_CLOSED_MEM_INSUFFICIENT",
		                   [XCB_CONN_CLOSED_REQ_LEN_EXCEED]   = "XCB_CONN_CLOSED_REQ_LEN_EXCEED",
		                   [XCB_CONN_CLOSED_PARSE_ERR]        = "XCB_CONN_CLOSED_PARSE_ERR",
		                   [XCB_CONN_CLOSED_INVALID_SCREEN]   = "XCB_CONN_CLOSED_INVALID_SCREEN"
		};

		fwm_log_error("Could not connect to the X server: %s (%u)", error_strings[error_code], error_code);
		fwm_exit(error_code);
	}

	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(fwm.conn));
	for (int i = 0; i < preferred_screen; i++)
		xcb_screen_next(&screen_iterator);

	fwm.screen = screen_iterator.data;
	fwm.root = fwm.screen->root;
}

void fwm_register_events(void) {
	xcb_generic_error_t *error = xcb_request_check(fwm.conn,
	                                               xcb_change_window_attributes_checked(fwm.conn, fwm.root,
	                                                                                    XCB_CW_EVENT_MASK,
	                                                                                    &(uint32_t){ ROOT_EVENT_MASK }));

	if (error) {
		fwm_log_error("Could not register for substructure redirection (%u). Is another window manager running?", error->error_code);
		fwm_exit(error->error_code);
	}
}

void fwm_exit(int status) {
	xcb_disconnect(fwm.conn);
	exit(status);
}
