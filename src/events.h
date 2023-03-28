#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xcb.h>
#include <xcb/xproto.h>

struct fwm_event_handlers {
	void (*map_request)(xcb_map_request_event_t*);
};

extern struct fwm_event_handlers fwm_event_handlers;

void fwm_initialize_event_handlers(void);
void fwm_handle_event(xcb_generic_event_t *event);
void fwm_event_map_request(xcb_map_request_event_t *event);

#endif
