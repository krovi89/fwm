#ifndef FWM_H
#define FWM_H

#include <stdio.h>
#include <stdbool.h>

#include <sys/un.h>

#include <xcb/xproto.h>

#include "events.h"
#include "keybinds.h"

#define FWM_MAX_MESSAGE_LEN 255

#define FWM_MAX_CLIENTS 30
#define FWM_CLIENT_TIMEOUT 5

#define FWM_ROOT_EVENT_MASK XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                            XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY

#define FWM_EXEC_SHELL "/bin/sh"

/* Holds the window manager's state */
struct fwm {
	xcb_connection_t *conn;
	int               conn_fd;
	xcb_screen_t     *screen;
	xcb_window_t      root;

	int                socket_fd;
	struct sockaddr_un socket_address;

	char *cache_dir;
	FILE *log_file;

	char *exec_shell;

	/* declared in keybinds.h */
	struct fwm_keybind *keybinds;
	struct fwm_keybind *current_position;
	size_t              max_keybind_id;
};

extern struct fwm fwm;

void fwm_initialize(void);
void fwm_initialize_socket(void);
char *fwm_initialize_cache(void);
void fwm_set_signal_handler(void (*handler)(int));
void fwm_signal_handler(int signal);
void fwm_connection_has_error(void);
void fwm_close_files(void);
void fwm_exit(int status);

#endif
