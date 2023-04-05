#ifndef KEYBINDS_H
#define KEYBINDS_H

#include <stdbool.h>

#include "actions.h"

struct fwm_keybind {
	size_t id;

	uint16_t keymask;
	uint8_t keycode;

	struct fwm_keybind *parent;
	struct fwm_keybind *child;
	struct fwm_keybind *next;
	struct fwm_keybind *previous;

	struct fwm_action *actions;
};

bool fwm_add_keybind(struct fwm_keybind *keybind);
void fwm_set_keybinds_position(struct fwm_keybind *keybind);
void fwm_grab_keybinds(struct fwm_keybind *keybind);
struct fwm_keybind *fwm_find_keybind(size_t id, struct fwm_keybind *current);
void fwm_free_keybind(struct fwm_keybind *keybind, bool keep_siblings);
void fwm_remove_keybind(struct fwm_keybind *keybind);
void fwm_remove_all_keybinds(void);

#endif