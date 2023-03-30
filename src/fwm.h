#ifndef FWM_H
#define FWM_H

#include <stdbool.h>

#include <sys/un.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define FWM_MAX_CLIENTS 30
#define FWM_CLIENT_TIMEOUT 5

#define ROOT_EVENT_MASK XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY

struct fwm {
	xcb_connection_t   *conn;
	int                 conn_fd;
	xcb_screen_t       *screen;
	xcb_window_t        root;

	int                 socket_fd;
	struct sockaddr_un  socket_address;
};

extern struct fwm fwm;

void fwm_initialize(void);
void fwm_initialize_socket(void);
void fwm_signal_handler(int signal);
void fwm_connection_has_error(void);
void fwm_exit(int status);

#endif
