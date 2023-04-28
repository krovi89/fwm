#ifndef EVENTS_H
#define EVENTS_H

#include <xcb/xproto.h>

void fwm_initialize_event_handlers(void);
void fwm_handle_event(xcb_generic_event_t *event);
void fwm_event_map_request(xcb_map_request_event_t *event);
void fwm_event_configure_request(xcb_configure_request_event_t *event);
void fwm_event_key_press(xcb_key_press_event_t *event);

#endif
