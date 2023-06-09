#ifndef FWM_H
#define FWM_H

#include <stdio.h>
#include <stdbool.h>

#include <poll.h>
#include <sys/un.h>

#include <xcb/xproto.h>

#include "events.h"
#include "keybinds.h"
#include "actions.h"
#include "files.h"

#define FWM_MAX_MESSAGE_LEN 255

#define FWM_MAX_CLIENTS 30
#define FWM_CLIENT_TIMEOUT 5

#define FWM_ROOT_EVENT_MASK XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY

#define FWM_EXEC_SHELL "/bin/sh"

struct fwm_env {
	char *home;
	char *xdg_data_home;

	char *data_dir;
	char *log_file;

	char *shell;
	char *exec_shell;
};

/* holds the window manager's state */
struct fwm {
	xcb_connection_t *conn;
	int               conn_fd;
	xcb_screen_t     *screen;
	xcb_window_t      root;

	int                socket_fd;
	struct sockaddr_un socket_address;

	struct pollfd *poll_fds;
	struct pollfd *clients;

	size_t  clients_num;
	time_t *client_connection_times;

	/* should be moved to an options struct */
	char *exec_shell;
	bool show_diagnostics;

	/* declared in keybinds.h */
	struct fwm_keybind *keybinds;
	struct fwm_keybind *current_position;
	size_t              max_keybind_id;

	/* array of action parsers */
	struct fwm_action *(*action_parsers[FWM_MAX_ACTION + 1])(const uint8_t*);
	bool (*action_validators[FWM_MAX_ACTION + 1])(const uint8_t **action, int *length);

	struct fwm_env env;
	struct fwm_files files;
};

extern struct fwm fwm;

void fwm_initialize(void);
void fwm_initialize_env(void);
void fwm_initialize_x(void);
void fwm_initialize_socket(void);
void fwm_initialize_poll_fds(void);
void fwm_initialize_clients(void);
void fwm_set_signal_handler(void (*handler)(int));
void fwm_signal_handler(int signal);
void fwm_connection_has_error(void);
void fwm_close_files(void);
void fwm_exit(int status);

#endif
