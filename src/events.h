#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xproto.h>

struct fwm_event_handlers {
	void (*map_request)(xcb_map_request_event_t*);
	void (*configure_request)(xcb_configure_request_event_t*);
	void (*key_press)(xcb_key_press_event_t*);
};

void fwm_initialize_event_handlers(void);
void fwm_handle_event(xcb_generic_event_t *event);
void fwm_event_map_request(xcb_map_request_event_t *event);
void fwm_event_configure_request(xcb_configure_request_event_t *event);
void fwm_event_key_press(xcb_key_press_event_t *event);

#endif
