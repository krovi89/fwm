#ifndef FWM_H
#define FWM_H

#include <xcb/xcb.h>

struct fwm {
	xcb_connection_t *conn;
	xcb_screen_t     *screen;
};

extern struct fwm fwm;

int fwm_initialize(void);
int fwm_connection_check_error(void);
void fwm_exit(int status);

#endif
