#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <poll.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/randr.h>

#include "fwm.h"
#include "events.h"
#include "messages.h"
#include "log.h"

struct fwm fwm;

static struct pollfd poll_fds[2 + FWM_MAX_CLIENTS];
static struct pollfd *clients = poll_fds + 2;
static int clients_num = 0;

int main(void) {
	fwm_initialize();
	fwm_initialize_socket();
	fwm_initialize_event_handlers();

	struct sigaction signal_action = {
		.sa_handler = fwm_signal_handler
	};

	sigaction(SIGINT, &signal_action, NULL);
	sigaction(SIGTERM, &signal_action, NULL);
	sigaction(SIGHUP, &signal_action, NULL);

	poll_fds[0].fd = fwm.conn_fd;
	poll_fds[1].fd = fwm.socket_fd;
	poll_fds[0].events = poll_fds[1].events = POLLIN;

	for (int i = 0; i < FWM_MAX_CLIENTS; i++) {
		clients[i].fd = -1;
		clients[i].events = POLLIN;
	}

	time_t client_times[FWM_MAX_CLIENTS] = {0};

	/* very oversized message buffer */
	unsigned char message[255];

	xcb_generic_event_t *event;
	for (;;) {
		if (poll(poll_fds, 2 + FWM_MAX_CLIENTS, -1) > 0) {
			for (int i = 0; i < clients_num; i++) {
				bool fd_has_error = clients[i].revents & (POLLERR | POLLNVAL | POLLHUP);
				bool has_timed_out = client_times[i] && client_times[i] + FWM_CLIENT_TIMEOUT < time(NULL);

				if (fd_has_error || has_timed_out) {
					close(clients[i].fd);

					if (i + 1 != clients_num) {
						memmove(&clients[i], &clients[i] + 1, sizeof (struct pollfd) * clients_num - (i + 1));
					}

					clients[--clients_num].fd = -1;

					continue;
				}

				if (clients[i].revents & POLLIN) {
					int length = recv(clients[i].fd, message, sizeof message, 0);

					/* The message must at least contain the header, and a request number.
					   Otherwise, it's not valid */
					if (length < (int) (sizeof message_header + 1)) continue;
					if (memcmp(message_header, message, sizeof message_header)) continue;

					fwm_process_message(message, length);

					/* Set the time to 0, so the client never times out */
					client_times[i] = 0;
				}
			}

			if (poll_fds[1].revents & POLLIN) {
				if (clients_num == FWM_MAX_CLIENTS) {
					int rejected_fd = accept(fwm.socket_fd, NULL, 0);
					close(rejected_fd);

					continue;
				}

				clients[clients_num].fd = accept(fwm.socket_fd, NULL, 0);
				client_times[clients_num++] = time(NULL);
			}

			if (poll_fds[0].revents & POLLIN) {
				while ((event = xcb_poll_for_event(fwm.conn))) {
					fwm_handle_event(event);
					free(event);
				}
			}
		}

		fwm_connection_has_error();
		xcb_flush(fwm.conn);
	}
}

void fwm_initialize(void) {
	/* socket_fd has to be initialized here, because fwm_connection_has_error
	   may call fwm_exit, in which case, file descriptor 0 would be closed. */
	fwm.socket_fd = -1;

	int preferred_screen;
	fwm.conn = xcb_connect(NULL, &preferred_screen);
	fwm_connection_has_error();
	fwm.conn_fd = xcb_get_file_descriptor(fwm.conn);

	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(fwm.conn));
	for (int i = 0; i < preferred_screen; i++) {
		xcb_screen_next(&screen_iterator);
	}

	fwm.screen = screen_iterator.data;
	fwm.root = fwm.screen->root;

	xcb_generic_error_t *error = xcb_request_check(fwm.conn,
	                                               xcb_change_window_attributes_checked(fwm.conn, fwm.root,
	                                                                                    XCB_CW_EVENT_MASK,
	                                                                                    &(uint32_t){ ROOT_EVENT_MASK }));
	if (error) {
		fwm_log_error_exit(EXIT_FAILURE, "Could not register for substructure redirection. Is another window manager running?\n");
	}
}

void fwm_initialize_socket(void) {
	if ((fwm.socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		fwm_log_error_exit(EXIT_FAILURE, "Socket creation failed.\n");
	}

	char *host_name;
	int display_number, screen_number;
	/* fwm_initialize already checks for errors with the display string */
	xcb_parse_display(NULL, &host_name, &display_number, &screen_number);

	fwm.socket_address.sun_family = AF_UNIX;
	int ret = snprintf(fwm.socket_address.sun_path, sizeof fwm.socket_address.sun_path,
	               "/tmp/fwm_%s_%i-%i", host_name, display_number, screen_number);

	free(host_name);

	if (ret > (int) sizeof fwm.socket_address.sun_path - 1) {
		fwm_log_warning("Socket path is too long.\n");
	} else if (ret < 0) {
		fwm_log_error_exit(EXIT_FAILURE, "Failed to write socket path.\n");
	}

	remove(fwm.socket_address.sun_path);

	if (bind(fwm.socket_fd, (struct sockaddr*) &fwm.socket_address, sizeof fwm.socket_address) == -1) {
		fwm_log_error_exit(EXIT_FAILURE, "Socket binding failed.\n");
	}

	if (listen(fwm.socket_fd, SOMAXCONN) == -1) {
		fwm_log_error_exit(EXIT_FAILURE, "Listening to the socket failed.\n");
	}
}

void fwm_signal_handler(int signal) {
	fwm_log_info("Signal %i received, exiting..", signal);
	fwm_exit(EXIT_SUCCESS);
}

void fwm_connection_has_error(void) {
	int error_code = xcb_connection_has_error(fwm.conn);
	if (error_code) {
		char *error_strings[XCB_CONN_CLOSED_FDPASSING_FAILED + 1] = {
		                   [XCB_CONN_ERROR]                   = "XCB_CONN_ERROR",
		                   [XCB_CONN_CLOSED_EXT_NOTSUPPORTED] = "XCB_CONN_CLOSED_EXT_NOTSUPPORTED",
		                   [XCB_CONN_CLOSED_MEM_INSUFFICIENT] = "XCB_CONN_CLOSED_MEM_INSUFFICIENT",
		                   [XCB_CONN_CLOSED_REQ_LEN_EXCEED]   = "XCB_CONN_CLOSED_REQ_LEN_EXCEED",
		                   [XCB_CONN_CLOSED_PARSE_ERR]        = "XCB_CONN_CLOSED_PARSE_ERR",
		                   [XCB_CONN_CLOSED_INVALID_SCREEN]   = "XCB_CONN_CLOSED_INVALID_SCREEN",
		                   [XCB_CONN_CLOSED_FDPASSING_FAILED] = "XCB_CONN_CLOSED_FDPASSING_FAILED",
		};

		fwm_log_error_exit(error_code, "Could not connect to the X server: %s (%u).\n", error_strings[error_code], error_code);
	}
}

void fwm_exit(int status) {
	for (int i = 0; i < clients_num; i++) {
		close(clients[i].fd);
	}

	close(fwm.socket_fd);
	remove(fwm.socket_address.sun_path);

	xcb_flush(fwm.conn);
	xcb_disconnect(fwm.conn);

	exit(status);
}
