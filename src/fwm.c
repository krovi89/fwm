#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "fwm.h"
#include "log.h"

struct fwm fwm;

int main(void) {
	fwm_initialize();
	fwm_register_events();

	fwm_exit(0);
}

void fwm_initialize(void) {
	int preferred_screen;
	fwm.conn = xcb_connect(NULL, &preferred_screen);
	fwm_connection_has_error();

	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(fwm.conn));
	for (int i = 0; i < preferred_screen; i++)
		xcb_screen_next(&screen_iterator);

	fwm.screen = screen_iterator.data;
	fwm.root = fwm.screen->root;

	fwm_initialize_socket();
}

void fwm_initialize_socket(void) {
	if ((fwm.socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
		fwm_log_error_exit(EXIT_FAILURE, "Socket creation failed.\n");

	char *host_name;
	int display_number, screen_number;
	/* don't need to check for errors in parsing the display string */
	/* because fwm_initialize() exits with XCB_CONN_CLOSED_PARSE_ERR if there are any */
	xcb_parse_display(NULL, &host_name, &display_number, &screen_number);

	fwm.socket_address.sun_family = AF_UNIX;
	int ret = snprintf(fwm.socket_address.sun_path, sizeof fwm.socket_address.sun_path,
	               "/tmp/fwm_%s_%i-%i", host_name, display_number, screen_number);

	free(host_name);

	if (ret > (int) sizeof fwm.socket_address.sun_path - 1)
		fwm_log_warning("Socket path is too long.\n");
	else if (ret < 0)
		fwm_log_error_exit(EXIT_FAILURE, "Failed to write socket path.\n");

	remove(fwm.socket_address.sun_path);

	if (bind(fwm.socket_fd, (struct sockaddr*) &fwm.socket_address, sizeof fwm.socket_address) == -1)
		fwm_log_error_exit(EXIT_FAILURE, "Socket binding failed.\n");

	if (listen(fwm.socket_fd, SOMAXCONN) == -1)
		fwm_log_error_exit(EXIT_FAILURE, "Listening to the socket failed.\n");

	if (fcntl(fwm.socket_fd, F_SETFL, fcntl(fwm.socket_fd, F_GETFL) | O_NONBLOCK) == -1)
		fwm_log_error_exit(EXIT_FAILURE, "Failed to set O_NONBLOCK on the socket file descriptor.\n");
}

void fwm_connection_has_error(void) {
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

		fwm_log_error_exit(error_code, "Could not connect to the X server: %s (%u).\n", error_strings[error_code], error_code);
	}
}

void fwm_register_events(void) {
	xcb_generic_error_t *error = xcb_request_check(fwm.conn,
	                                               xcb_change_window_attributes_checked(fwm.conn, fwm.root,
	                                                                                    XCB_CW_EVENT_MASK,
	                                                                                    &(uint32_t){ ROOT_EVENT_MASK }));

	if (error)
		fwm_log_error_exit(EXIT_FAILURE, "Could not register for substructure redirection. Is another window manager running?\n");
}

void fwm_exit(int status) {
	close(fwm.socket_fd);
	remove(fwm.socket_address.sun_path);
	xcb_disconnect(fwm.conn);
	exit(status);
}
