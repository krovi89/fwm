#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>
#include <stdbool.h>

#include <xcb/xproto.h>

struct fwm_action *fwm_parse_action_execute(const uint8_t *actions);
void fwm_run_action_execute(void *args, xcb_window_t window);
void fwm_free_action_execute(struct fwm_action *action);
bool fwm_validate_action_execute(const uint8_t **action, int *length);

#endif
