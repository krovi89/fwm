#ifndef MESSAGES_H
#define MESSAGES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define FWM_REQUEST_NOTHING 0
#define FWM_REQUEST_EXIT 1
#define FWM_REQUEST_ADD_KEYBIND 2
#define FWM_REQUEST_REMOVE_KEYBIND 3
#define FWM_REQUEST_REMOVE_ALL_KEYBINDS 4
#define FWM_REQUEST_GET_KEYBIND_ID 5

#define FWM_KEYBIND_ADDED 1
#define FWM_KEYBIND_REMOVED 2
#define FWM_KEYBIND_REMOVED_ALL 3
#define FWM_KEYBIND_FOUND 4
#define FWM_KEYBIND_ALREADY_EXISTS 5
#define FWM_KEYBIND_INVALID_ID 6
#define FWM_KEYBIND_NOT_FOUND 7

#define FWM_INVALID_REQUEST 254
#define FWM_UNRECOGNIZED_REQUEST 255

extern const unsigned char message_header[3];

void fwm_handle_request(int client_fd, uint8_t type, const uint8_t *message, int length);

void fwm_parse_request_add_keybind(int client_fd, const uint8_t *message, int length);
uint8_t fwm_handle_request_add_keybind(uint8_t parents_num, uint8_t actions_num,
                                       const uint8_t *parents, const uint8_t *actions,
                                       size_t *id);

void fwm_parse_request_remove_keybind(int client_fd, const uint8_t *message, int length);
uint8_t fwm_handle_request_remove_keybind(size_t id);

void fwm_parse_request_get_keybind_id(int client_fd, const uint8_t *message, int length);
uint8_t fwm_handle_request_get_keybind_id(uint8_t parents_num, const uint8_t *parents,
                                          size_t *id);

void fwm_compose_send_response(int client_fd, uint8_t type, void *details);

struct fwm_keybind *fwm_parse_keybind(uint8_t parents_num, const uint8_t *parents,
                                      struct fwm_keybind **keybind, bool assign_id);
struct fwm_action *fwm_parse_action(uint8_t actions_num, const uint8_t *actions);

#endif
