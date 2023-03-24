/* stdlib */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* XCB */
#include <xcb/xcb.h>
#include <xcb/xproto.h>

/* fwm */
#include "fwm.h"

struct fwm fwm;

int main(void) {
	int initialize_error = fwm_initialize();
	if (initialize_error)
		fwm_exit(initialize_error);

	fwm_exit(0);
}

int fwm_initialize(void) {
	int preferred_screen;
	fwm.conn = xcb_connect(NULL, &preferred_screen);

	int connection_error = fwm_connection_check_error();
	if (connection_error)
		return connection_error;

	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(fwm.conn));
	for (int i = 0; i < preferred_screen; i++) xcb_screen_next(&screen_iterator);
	xcb_screen_next(&screen_iterator);
	fwm.screen = screen_iterator.data;

	return 0;
}

int fwm_connection_check_error(void) {
	int error = xcb_connection_has_error(fwm.conn);
	if (error) {
		char *error_strings[XCB_CONN_CLOSED_INVALID_SCREEN + 1] = {
			[XCB_CONN_ERROR] = "XCB_CONN_ERROR",
			[XCB_CONN_CLOSED_EXT_NOTSUPPORTED] = "XCB_CONN_CLOSED_EXT_NOTSUPPORTED",
			[XCB_CONN_CLOSED_MEM_INSUFFICIENT] = "XCB_CONN_CLOSED_MEM_INSUFFICIENT",
			[XCB_CONN_CLOSED_REQ_LEN_EXCEED] = "XCB_CONN_CLOSED_REQ_LEN_EXCEED",
			[XCB_CONN_CLOSED_PARSE_ERR] = "XCB_CONN_CLOSED_PARSE_ERR",
			[XCB_CONN_CLOSED_INVALID_SCREEN] = "XCB_CONN_CLOSED_INVALID_SCREEN"
		};
		fprintf(stderr, "ERROR: Could not connect to the X server: %s (%u)\n", error_strings[error], error);
	}
	return error;
}

void fwm_exit(int status) {
	xcb_disconnect(fwm.conn);
	exit(status);
}
