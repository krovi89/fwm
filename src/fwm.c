#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "fwm.h"
#include "events.h"
#include "messages.h"
#include "keybinds.h"
#include "actions.h"
#include "files.h"
#include "log.h"

struct fwm fwm;

int main(void) {
	fwm_initialize();

	uint8_t message[FWM_MAX_MESSAGE_LEN];
	for (;;) {
		if (poll(fwm.poll_fds, 2 + FWM_MAX_CLIENTS, -1) > 0) {
			for (size_t i = 0; i < fwm.clients_num; i++) {
				bool client_has_error = fwm.clients[i].revents & (POLLERR | POLLNVAL | POLLHUP);
				/* Has it been longer than FWM_CLIENT_TIMEOUT seconds since the client
				   established the connection, without sending a valid message? */
				bool client_timed_out = fwm.client_connection_times[i] && time(NULL) - fwm.client_connection_times[i] > FWM_CLIENT_TIMEOUT;

				/* clean up clients */
				if (client_has_error || client_timed_out) {
					if (client_has_error)
						FWM_DLOG("Removing client %zu: Poll error.\n", i);
					else if (client_timed_out)
						FWM_DLOG("Removing client %zu: Timed out.\n", i);

					close(fwm.clients[i].fd);

					/* no reason to memmove if the client
					   we're removing is the last one */
					if (i + 1 != fwm.clients_num)
						memmove(&fwm.clients[i], &fwm.clients[i] + 1, sizeof (struct pollfd) * fwm.clients_num - (i + 1));

					fwm.clients[--fwm.clients_num].fd = -1;

					continue;
				}

				/* Read messages from valid clients */
				if (fwm.clients[i].revents & POLLIN) {
					int message_length = recv(fwm.clients[i].fd, message, sizeof message, 0);
					FWM_DLOG("Received message of length %i from client %zu.\n", message_length, i);

					/* The message must at least contain the header, and a request number.
					   Otherwise, it's not valid */
					if (message_length < (int)(sizeof message_header + 1)) continue;
					if (memcmp(message_header, message, sizeof message_header)) continue;

					int request_length = message_length - (sizeof message_header + 1);
					uint8_t request_type = *(message + sizeof message_header);
					const uint8_t *request_message = message + (sizeof message_header + 1);

					FWM_DLOG("Request type: %u.\n", request_type);

					fwm_handle_request(fwm.clients[i].fd, request_type, request_message, request_length);

					/* Disable the timeout for this client */
					fwm.client_connection_times[i] = 0;
				}
			}

			/* Establishing connections with new clients */
			if (fwm.poll_fds[1].revents & POLLIN) {
				if (fwm.clients_num == FWM_MAX_CLIENTS) {
					close(accept(fwm.socket_fd, NULL, 0));
					FWM_ELOG("Rejected client: Maximum number of clients already connected");
					continue;
				}

				FWM_DLOG("Adding client number %zu.\n", fwm.clients_num);

				fwm.clients[fwm.clients_num].fd = accept(fwm.socket_fd, NULL, 0);
				/* Set the time of connection for this client */
				fwm.client_connection_times[fwm.clients_num++] = time(NULL);
			}

			if (fwm.poll_fds[0].revents & POLLIN) {
				xcb_generic_event_t *event;
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
	fwm.show_diagnostics = false;

	fwm_initialize_env();
	fwm_initialize_files();

	FWM_DLOG("Initializing fwm.\n", fwm.exec_shell);

	fwm_set_signal_handler(fwm_signal_handler);

	if (fwm.env.exec_shell) {
		fwm.exec_shell = fwm.env.exec_shell;
	} else {
		if (fwm.env.shell)
			fwm.exec_shell = fwm.env.shell;
		else
			fwm.exec_shell = FWM_EXEC_SHELL;
	}

	FWM_DLOG("Execution shell set to \"%s\".\n", fwm.exec_shell);

	fwm.socket_fd = -1;

	fwm_initialize_x();
	fwm_initialize_socket();
	fwm_initialize_poll_fds();
	fwm_initialize_clients();

	fwm.keybinds = NULL;
	fwm.current_position = NULL;
	fwm.max_keybind_id = 0;

	fwm_initialize_actions();
	fwm_initialize_action_validators();
}

void fwm_initialize_env(void) {
	fwm.env.home = getenv("HOME");
	fwm.env.xdg_data_home = getenv("XDG_DATA_HOME");
	fwm.env.data_dir = getenv("FWM_DATA_DIR");
	fwm.env.log_file = getenv("FWM_LOG_FILE");
	fwm.env.shell = getenv("SHELL");
	fwm.env.exec_shell = getenv("FWM_EXEC_SHELL");
}

void fwm_initialize_x(void) {
	int preferred_screen;
	fwm.conn = xcb_connect(NULL, &preferred_screen);
	fwm_connection_has_error();
	fwm.conn_fd = xcb_get_file_descriptor(fwm.conn);

	/* Get the preferred screen */
	xcb_screen_iterator_t screen_iterator = xcb_setup_roots_iterator(xcb_get_setup(fwm.conn));
	for (int i = 0; i < preferred_screen; i++)
		xcb_screen_next(&screen_iterator);

	fwm.screen = screen_iterator.data;
	fwm.root = fwm.screen->root;

	xcb_generic_error_t *error = xcb_request_check(fwm.conn,
                                                       xcb_change_window_attributes_checked(fwm.conn, fwm.root,
                                                                                            XCB_CW_EVENT_MASK,
                                                                                            &(uint32_t){ FWM_ROOT_EVENT_MASK }));
	if (error) {
		FWM_ELOG("Could not register for substructure redirection. Is another window manager running?\n");
		fwm_exit(EXIT_FAILURE);
	}
}

void fwm_initialize_socket(void) {
	if ((fwm.socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
		FWM_ELOG("Socket creation failed.\n");
		fwm_exit(EXIT_FAILURE);
	}

	char *host_name;
	int display_number, screen_number;
	/* fwm_initialize_x already checks for errors with the display string */
	xcb_parse_display(NULL, &host_name, &display_number, &screen_number);

	fwm.socket_address.sun_family = AF_UNIX;
	int ret = snprintf(fwm.socket_address.sun_path, sizeof fwm.socket_address.sun_path,
	                   "/tmp/fwm_%s_%i-%i", host_name, display_number, screen_number);

	free(host_name);

	FWM_DLOG("Socket path set to \"%s\".\n", fwm.socket_address.sun_path);

	if (ret > (int)(sizeof fwm.socket_address.sun_path - 1)) {
		fwm_log(FWM_LOG_WARNING, "Socket path is too long.\n");
	} else if (ret < 0) {
		FWM_ELOG("Failed to write socket path.\n");
		fwm_exit(EXIT_FAILURE);
	}

	remove(fwm.socket_address.sun_path);

	if (bind(fwm.socket_fd, (struct sockaddr*)(&fwm.socket_address), sizeof fwm.socket_address) == -1) {
		FWM_ELOG("Socket binding failed.\n");
		fwm_exit(EXIT_FAILURE);
	}

	if (listen(fwm.socket_fd, SOMAXCONN) == -1) {
		FWM_ELOG("Listening to the socket failed.\n");
		fwm_exit(EXIT_FAILURE);
	}
}

void fwm_initialize_poll_fds(void) {
	fwm.poll_fds = calloc(2 + FWM_MAX_CLIENTS, sizeof (struct pollfd));
	if (!fwm.poll_fds) {
		FWM_ELOG("Failed to allocate poll structures.\n");
		fwm_exit(EXIT_FAILURE);
	}

	fwm.poll_fds[0].fd = fwm.conn_fd;
	fwm.poll_fds[1].fd = fwm.socket_fd;
	fwm.poll_fds[0].events = fwm.poll_fds[1].events = POLLIN;
}

void fwm_initialize_clients(void) {
	fwm.clients = fwm.poll_fds + 2;
	for (size_t i = 0; i < FWM_MAX_CLIENTS; i++) {
		fwm.clients[i].fd = -1;
		fwm.clients[i].events = POLLIN;
	}

	fwm.clients_num = 0;
	fwm.client_connection_times = calloc(FWM_MAX_CLIENTS, sizeof (time_t));
	if (!fwm.client_connection_times) {
		FWM_ELOG("Failed to allocate memory for client connection times.\n");
		fwm_exit(EXIT_FAILURE);
	}
}

void fwm_set_signal_handler(void (*handler)(int)) {
	struct sigaction signal_action = {
		.sa_handler = handler
	};

	sigaction(SIGINT, &signal_action, NULL);
	sigaction(SIGTERM, &signal_action, NULL);
	sigaction(SIGHUP, &signal_action, NULL);
	sigaction(SIGCHLD, &signal_action, NULL);
}

void fwm_signal_handler(int signal) {
	if (signal == SIGCHLD) {
		while (waitpid(-1, NULL, WNOHANG) > 0);
	} else {
		FWM_DLOG("Signal %i received.\n", signal);
		fwm_exit(EXIT_SUCCESS);
	}
}

void fwm_connection_has_error(void) {
	int error_code = xcb_connection_has_error(fwm.conn);
	if (error_code) {
		char *error_strings[XCB_CONN_CLOSED_FDPASSING_FAILED + 1] = {
			[XCB_CONN_ERROR]                   = "Socket/pipe/stream error",
			[XCB_CONN_CLOSED_EXT_NOTSUPPORTED] = "Extension unsupported",
			[XCB_CONN_CLOSED_MEM_INSUFFICIENT] = "Insufficient memory",
			[XCB_CONN_CLOSED_REQ_LEN_EXCEED]   = "Request length exceeded",
			[XCB_CONN_CLOSED_PARSE_ERR]        = "Failed to parse display string",
			[XCB_CONN_CLOSED_INVALID_SCREEN]   = "Invalid screen",
			[XCB_CONN_CLOSED_FDPASSING_FAILED] = "File descriptor passing failed",
		};

		FWM_ELOG("Connecting to the X server failed: %s (%u).\n", error_strings[error_code], error_code);
		fwm_exit(error_code);
	}
}

void fwm_close_files(void) {
	if (fwm.poll_fds)
		for (size_t i = 0; i < 2 + fwm.clients_num; i++) {
			close(fwm.poll_fds[i].fd);
			fwm.poll_fds[i].fd = -1;
		}

	if (fwm.files.log_file) {
		fclose(fwm.files.log_file);
		fwm.files.log_file = NULL;
	}
}

void fwm_exit(int status) {
	FWM_DLOG("Exiting fwm.\n");

	if (fwm.keybinds)
		fwm_remove_all_keybinds();

	fwm_close_files();

	free(fwm.poll_fds);
	free(fwm.client_connection_times);

	remove(fwm.socket_address.sun_path);

	if (fwm.conn) {
		xcb_flush(fwm.conn);
		xcb_disconnect(fwm.conn);
	}

	exit(status);
}
