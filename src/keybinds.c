#include <stdlib.h>
#include <stdbool.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "keybinds.h"
#include "fwm.h"

bool fwm_add_keybind(struct fwm_keybind *keybind) {
	if (!fwm.keybinds) {
		fwm.keybinds = keybind;
		fwm.current_position = keybind;
		xcb_grab_key(fwm.conn, 0, fwm.root, keybind->keymask, keybind->keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		return true;
	}

	struct fwm_keybind *a = fwm.keybinds;
	struct fwm_keybind *b = keybind;
	for (;;) {
		if (a->keycode == b->keycode && a->keymask == b->keymask) {
			if (!a->child) {
				fwm_free_keybind(b, true);
				return false;
			}

			struct fwm_keybind *temp = b->child;
			free(b);
			b = temp;
			b->parent = a;

			a = a->child;
		} else {
			if (!a->next) {
				b->previous = a;
				a->next = b;

				if (a == fwm.current_position)
					xcb_grab_key(fwm.conn, 0, fwm.root, b->keymask, b->keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

				break;
			}
			a = a->next;
		}
	}

	return true;
}

void fwm_set_keybinds_position(struct fwm_keybind *position) {
	fwm.current_position = position;
	xcb_ungrab_key(fwm.conn, XCB_GRAB_ANY, fwm.root, XCB_MOD_MASK_ANY);
	if (position)
		fwm_grab_keybinds(position);
}

void fwm_grab_keybinds(struct fwm_keybind *keybinds) {
	while (keybinds) {
		xcb_grab_key(fwm.conn, 0, fwm.root, keybinds->keymask, keybinds->keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
		keybinds = keybinds->next;
	}
}

struct fwm_keybind *fwm_find_keybind(size_t id, struct fwm_keybind *current) {
	if (current->id == id)
		return current;

	struct fwm_keybind *found = NULL;
	if (current->child)
		if ((found = fwm_find_keybind(id, current->child)))
			return found;
	if (current->next)
		if ((found = fwm_find_keybind(id, current->next)))
			return found;

	return NULL;
}

void fwm_free_keybind(struct fwm_keybind *keybind, bool keep_siblings) {
	if (keybind->child)
		fwm_free_keybind(keybind->child, false);
	if (!keep_siblings && keybind->next)
		fwm_free_keybind(keybind->next, false);

	while (keybind->actions) {
		struct fwm_action *temp = keybind->actions->next;
		free(keybind->actions);
		keybind->actions = temp;
	}

	free(keybind);
}

void fwm_remove_keybind(struct fwm_keybind *keybind) {
	for (;;) {
		if (keybind->next) {
			if (keybind->previous) {
				/* if there's a following and a previous binding,
				   then we just have to unlink */
				keybind->previous->next = keybind->next;
				keybind->next->previous = keybind->previous;
			} else {
				/* unlink */
				keybind->next->previous = NULL;

				if (keybind->parent) {
					/* if there's a parent and a previous binding,
					   then we're deleting the first child of the keybind. */
					keybind->parent->child = keybind->next;
				} else {
					/* if there's no parent, and no previous binding,
					   then we're deleting the first root binding. */
					fwm.keybinds = keybind->next;
				}
			}
			break;
		} else if (keybind->previous) {
			/* if there's only a previous binding,
			   then we just have to unlink. */
			keybind->previous->next = NULL;
			break;
		} else {
			/* if there's no previous or following keybind, then either the parent keybind has no more children,
			   so we should delete it as well, or this is the lone root keybind (no parents). */
			if (keybind->parent) {
				keybind = keybind->parent;
			} else {
				fwm.keybinds = NULL;
				break;
			}

		}
	}

	fwm_set_keybinds_position(fwm.keybinds);
	fwm_free_keybind(keybind, true);
}

void fwm_remove_all_keybinds(void) {
	fwm_free_keybind(fwm.keybinds, false);
	fwm.keybinds = NULL;
	fwm_set_keybinds_position(NULL);
}