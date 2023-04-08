#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <sys/socket.h>

#include "messages.h"
#include "fwm.h"
#include "actions.h"
#include "keybinds.h"
#include "log.h"

const uint8_t message_header[3] = { 'f', 'w', 'm' };

void fwm_handle_request(int client_fd, uint8_t type, const uint8_t *message, int length) {
	switch (type) {
		case FWM_REQUEST_NOTHING:
			break;
		case FWM_REQUEST_EXIT:
			fwm_log_info("Received FWM_REQUEST_EXIT, exiting..");
			fwm_exit(EXIT_SUCCESS);
			break;
		case FWM_REQUEST_ADD_KEYBIND:
			fwm_parse_request_add_keybind(client_fd, message, length);
			break;
		case FWM_REQUEST_REMOVE_KEYBIND:
			fwm_parse_request_remove_keybind(client_fd, message, length);
			break;
		default:
			fwm_compose_send_reply(client_fd, FWM_RESPONSE_UNRECOGNIZED_REQUEST, NULL);
			break;
	}
}

void fwm_parse_request_add_keybind(int client_fd, const uint8_t *message, int length) {
	if (length < 2) return;

	uint8_t parents_num = *message++;
	uint8_t actions_num = *message++;

	int minimum_length = 2 + ((sizeof (uint16_t) + 1) * (parents_num + 1)) + actions_num;
	if (length < minimum_length) {
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_INVALID_REQUEST, NULL);
		return;
	}

	uint16_t keymask = *(uint16_t*)message;
	message += sizeof (uint16_t);
	uint8_t keycode = *(message++);

	const uint8_t *parents = message;
	message += (sizeof (uint16_t) + 1) * parents_num;

	const uint8_t *actions = message;

	size_t id = 0;
	bool ret = fwm_handle_request_add_keybind(parents_num, actions_num,
	                                          keymask, keycode,
	                                          parents, actions, &id);

	if (ret)
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_SUCCESS_KEYBIND_ADDED, &id);
	else
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_FAILURE_KEYBIND_EXISTS, NULL);
}

void fwm_parse_request_remove_keybind(int client_fd, const uint8_t *message, int length) {
	if (length < (int)sizeof (size_t)) {
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_INVALID_REQUEST, NULL);
		return;
	}

	size_t id = *(size_t*)message;
	if (fwm_handle_request_remove_keybind(id))
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_SUCCESS_KEYBIND_REMOVED, &id);
	else
		fwm_compose_send_reply(client_fd, FWM_RESPONSE_FAILURE_KEYBIND_INVALID_ID, NULL);
}

bool fwm_handle_request_add_keybind(uint8_t parents_num, uint8_t actions_num,
                                    uint16_t keymask, uint8_t keycode,
                                    const uint8_t *parents, const uint8_t *actions,
                                    size_t *id)
{
	struct fwm_keybind *keybind = calloc(1, sizeof (struct fwm_keybind));
	struct fwm_keybind *keybind_root = keybind;
	for (int i = 0; i < parents_num; i++) {
		keybind->id = fwm.max_keybind_id++;

		keybind->keymask = *(uint16_t*)parents;
		parents += sizeof (uint16_t);
		keybind->keycode = *parents++;

		keybind->child = calloc(1, sizeof (struct fwm_keybind));
		keybind->child->parent = keybind;
		keybind = keybind->child;
	}

	keybind->id = fwm.max_keybind_id++;

	keybind->keycode = keycode;
	keybind->keymask = keymask;

	struct fwm_action *action = calloc(1, sizeof (struct fwm_action));
	keybind->actions = action;
	for (int i = 0; i < actions_num; i++) {
		uint8_t action_type = *actions++;
		switch (action_type) {
			case FWM_ACTION_CLOSE_FOCUSED:
				action->run = fwm_action_close_focused;
				break;
			default:
				break;
		}

		if (i + 1 != actions_num) {
			action->next = calloc(1, sizeof (struct fwm_action));
			action = action->next;
		}
	}

	if (!fwm_add_keybind(keybind_root)) return false;

	*id = keybind->id;
	return true;
}

bool fwm_handle_request_remove_keybind(size_t id) {
	struct fwm_keybind *keybind = fwm_find_keybind(id, fwm.keybinds);
	if (!keybind)
		return false;

	fwm_remove_keybind(keybind);
	return true;
}

void fwm_compose_send_reply(int client_fd, uint8_t reply_type, void *details) {
	static uint8_t message[FWM_MAX_MESSAGE_LEN] = {0};
	memcpy(message, message_header, sizeof message_header);

	uint8_t *position = message + sizeof message_header;
	size_t message_length = sizeof message_header;

	memcpy(position++, &reply_type, 1);
	message_length++;

	switch (reply_type) {
		case FWM_RESPONSE_SUCCESS_KEYBIND_ADDED:
		case FWM_RESPONSE_SUCCESS_KEYBIND_REMOVED:
			memcpy(position, (size_t*)details, sizeof (size_t));
			message_length += sizeof (size_t);
			break;
	}

	send(client_fd, message, message_length, MSG_NOSIGNAL);
}
