#ifndef CLOSE_FOCUSED_H
#define CLOSE_FOCUSED_H

#include <xcb/xproto.h>

#include "../fwm.h"

struct fwm_action *fwm_parse_action_close_focused(const uint8_t *actions);
void fwm_run_action_close_focused(void *args, xcb_window_t window);
void fwm_free_action_close_focused(struct fwm_action *action);
bool fwm_validate_action_close_focused(const uint8_t **action_ptr, int *length_ptr);

#endif
