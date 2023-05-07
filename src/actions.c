#include <stdio.h>
#include <stdlib.h>

#include <xcb/xproto.h>

#include "actions.h"
#include "fwm.h"

#include "actions/close_focused.h"
#include "actions/execute.h"

void fwm_initialize_actions(void) {
	fwm.action_parsers[FWM_ACTION_CLOSE_FOCUSED] = fwm_parse_action_close_focused;
	fwm.action_parsers[FWM_ACTION_EXECUTE] = fwm_parse_action_execute;
}

void fwm_initialize_action_validators(void) {
	fwm.action_validators[FWM_ACTION_CLOSE_FOCUSED] = fwm_validate_action_close_focused;
	fwm.action_validators[FWM_ACTION_EXECUTE] = fwm_validate_action_execute;
}

void fwm_free_actions(struct fwm_action *actions) {
	while (actions) {
		struct fwm_action *temp = actions->next;
		actions->free(actions);
		actions = temp;
	}
}
