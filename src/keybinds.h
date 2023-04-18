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

bool fwm_assimilate_keybind(struct fwm_keybind *keybind);

void fwm_set_keybinds_position(struct fwm_keybind *keybind);
void fwm_grab_keybinds(const struct fwm_keybind *keybind);

struct fwm_keybind *fwm_find_keybind_by_id(size_t id, struct fwm_keybind *current);
struct fwm_keybind *fwm_find_keybind_by_keys(struct fwm_keybind *keybind, struct fwm_keybind *current);

struct fwm_keybind *fwm_create_keybind(uint16_t keymask, uint8_t keycode, size_t id);

void fwm_free_keybind(struct fwm_keybind *keybind, bool keep_siblings);
void fwm_remove_keybind(struct fwm_keybind *keybind);
void fwm_remove_all_keybinds(void);

#endif
