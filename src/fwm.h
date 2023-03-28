#ifndef FWM_H
#define FWM_H

#include <stdbool.h>

#include <sys/un.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#define ROOT_EVENT_MASK XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT | \
                        XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY

struct fwm {
	xcb_connection_t   *conn;
	xcb_screen_t       *screen;
	xcb_window_t        root;

	int                 conn_fd;

	int                 socket_fd;
	struct sockaddr_un  socket_address;
};

extern struct fwm fwm;

void fwm_initialize(void);
void fwm_initialize_socket(void);
void fwm_connection_has_error(void);
void fwm_exit(int status);

#endif
