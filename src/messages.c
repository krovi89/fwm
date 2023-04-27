#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <sys/socket.h>

#include "messages.h"
#include "fwm.h"
#include "actions.h"
#include "keybinds.h"
#include "log.h"

const uint8_t message_header[3] = { 'f', 'w', 'm' };

void fwm_handle_request(int client_fd, uint8_t type, const uint8_t *message, int length) {
	switch (type) {
		case FWM_NO_REQUEST:
			break;
		case FWM_REQUEST_EXIT:
			fwm_log_info("Received FWM_REQUEST_EXIT, exiting..\n");
			fwm_exit(EXIT_SUCCESS);
			break;
		case FWM_REQUEST_KEYBIND_ADD:
			fwm_parse_request_add_keybind(client_fd, message, length);
			break;
		case FWM_REQUEST_KEYBIND_REMOVE:
			fwm_parse_request_remove_keybind(client_fd, message, length);
			break;
		case FWM_REQUEST_KEYBIND_REMOVE_ALL:
			if (fwm.keybinds) fwm_remove_all_keybinds();
			fwm_compose_send_response(client_fd, FWM_SUCCESS_KEYBIND_REMOVED_ALL, NULL);
			break;
		case FWM_REQUEST_KEYBIND_GET_ID:
			fwm_parse_request_get_keybind_id(client_fd, message, length);
			break;
		default:
			fwm_compose_send_response(client_fd, FWM_FAILURE_UNRECOGNIZED_REQUEST, NULL);
			break;
	}
}

void fwm_parse_request_add_keybind(int client_fd, const uint8_t *message, int length) {
	uint8_t parents_num = *message++;
	uint8_t actions_num = *message++;

	const uint8_t *parents = message;
	message += (sizeof (uint16_t) + 1) * (parents_num + 1);
	const uint8_t *actions = message;

	size_t id = 0;
	uint8_t response = fwm_handle_request_add_keybind(parents_num, actions_num,
	                                                  parents, actions, &id);

	fwm_compose_send_response(client_fd, response, &id);
}

uint8_t fwm_handle_request_add_keybind(uint8_t parents_num, uint8_t actions_num,
                                       const uint8_t *parents, const uint8_t *actions,
                                       size_t *id) {
	struct fwm_keybind *keybind = NULL;
	struct fwm_keybind *tree = fwm_parse_keybind(parents_num, parents,
	                                             &keybind, true);

	keybind->actions = fwm_parse_action(actions_num, actions);

	if (!fwm_assimilate_keybind(tree))
		return FWM_FAILURE_KEYBIND_ALREADY_EXISTS;

	*id = keybind->id;
	return FWM_SUCCESS_KEYBIND_ADDED;
}

void fwm_parse_request_remove_keybind(int client_fd, const uint8_t *message, int length) {
	size_t id = *(size_t*)(message);
	uint8_t response = fwm_handle_request_remove_keybind(id);

	fwm_compose_send_response(client_fd, response, NULL);
}

uint8_t fwm_handle_request_remove_keybind(size_t id) {
	struct fwm_keybind *keybind = fwm_find_keybind_by_id(id, fwm.keybinds);
	if (!keybind)
		return FWM_FAILURE_KEYBIND_INVALID_ID;

	fwm_remove_keybind(keybind);
	return FWM_SUCCESS_KEYBIND_REMOVED;
}

void fwm_parse_request_get_keybind_id(int client_fd, const uint8_t *message, int length) {
	uint8_t parents_num = *message++;

	const uint8_t *parents = message;

	size_t id = 0;
	uint8_t response = fwm_handle_request_get_keybind_id(parents_num, parents,
	                                                     &id);

	fwm_compose_send_response(client_fd, response, &id);
}

uint8_t fwm_handle_request_get_keybind_id(uint8_t parents_num, const uint8_t *parents,
                                          size_t *id) {
	struct fwm_keybind *tree = fwm_parse_keybind(parents_num, parents,
	                                             NULL, false);

	struct fwm_keybind *found = fwm_find_keybind_by_keys(tree, fwm.keybinds);
	uint8_t ret = FWM_FAILURE_KEYBIND_NOT_FOUND;
	if (found) {
		ret = FWM_SUCCESS_KEYBIND_FOUND;
		*id = found->id;
	}

	fwm_free_keybind(tree, true);
	return ret;
}

void fwm_compose_send_response(int client_fd, uint8_t response_type, void *details) {
	static uint8_t message[FWM_MAX_MESSAGE_LEN] = {0};
	memcpy(message, message_header, sizeof message_header);

	uint8_t *position = message + sizeof message_header;
	size_t message_length = sizeof message_header;

	memcpy(position++, &response_type, 1);
	message_length++;

	switch (response_type) {
		case FWM_SUCCESS_KEYBIND_ADDED:
		case FWM_SUCCESS_KEYBIND_FOUND:
			memcpy(position, (size_t*)(details), sizeof (size_t));
			message_length += sizeof (size_t);
			break;
	}

	send(client_fd, message, message_length, MSG_NOSIGNAL);
}

struct fwm_keybind *fwm_parse_keybind(uint8_t parents_num, const uint8_t *parents,
                                      struct fwm_keybind **keybind, bool assign_id) {
	struct fwm_keybind *base = NULL, *tree = NULL;
	for (uint8_t i = 0; i < parents_num + 1; i++) {
		uint16_t keymask = *(uint16_t*)(parents);
		parents += sizeof (uint16_t);
		uint16_t keycode = *parents++;

		if (!tree) {
			tree = fwm_create_keybind(keymask, keycode, assign_id ? ++fwm.max_keybind_id : 0);
			base = tree;
			continue;
		}

	 	tree->child = fwm_create_keybind(keymask, keycode, assign_id ? ++fwm.max_keybind_id : 0);
	 	tree->child->parent = tree;
	 	tree = tree->child;
	}

	if (keybind) *keybind = tree;
	return base;
}

struct fwm_action *fwm_parse_action(uint8_t actions_num, const uint8_t *actions) {
	struct fwm_action *action = NULL;
	for (int i = 0; i < actions_num; i++) {
		uint8_t action_type = *actions++;

		if (!action) {
			action = calloc(1, sizeof (struct fwm_action));
		} else {
			action->next = calloc(1, sizeof (struct fwm_action));
			action = action->next;
		}

		action->type = action_type;
		switch (action_type) {
		 	case FWM_ACTION_CLOSE_FOCUSED:
		 		action->run = fwm_action_close_focused;
		 		break;
		 	default:
		 		break;
		}
	}

	return action;
}
