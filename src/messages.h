#ifndef MESSAGES_H
#define MESSAGES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FWM_REQUEST_NOTHING 0
#define FWM_REQUEST_EXIT 1
#define FWM_REQUEST_ADD_KEYBIND 2
#define FWM_REQUEST_REMOVE_KEYBIND 3

#define FWM_RESPONSE_SUCCESS_KEYBIND_ADDED 1
#define FWM_RESPONSE_SUCCESS_KEYBIND_REMOVED 2
#define FWM_RESPONSE_FAILURE_KEYBIND_EXISTS 3
#define FWM_RESPONSE_FAILURE_KEYBIND_INVALID_ID 4
#define FWM_RESPONSE_INVALID_REQUEST 254
#define FWM_RESPONSE_UNRECOGNIZED_REQUEST 255

extern const unsigned char message_header[3];

void fwm_handle_request(int client_fd, uint8_t type, const uint8_t *message, int length);
void fwm_parse_request_add_keybind(int client_fd, const uint8_t *message, int length);
void fwm_parse_request_remove_keybind(int client_fd, const uint8_t *message, int length);
bool fwm_handle_request_add_keybind(uint8_t parents_num, uint8_t actions_num,
                                    uint16_t keymask, uint8_t keycode,
                                    const uint8_t *parents, const uint8_t *actions,
                                    size_t *id);
bool fwm_handle_request_remove_keybind(size_t id);
void fwm_compose_send_reply(int client_fd, uint8_t type, void *details);

#endif
