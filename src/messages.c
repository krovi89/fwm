#include "messages.h"

const unsigned char message_header[3] = { 'f', 'w', 'm' };

void fwm_process_message(unsigned char *message, int length) {
	message += sizeof message_header;
	unsigned char message_type = *message++;

	switch (message_type) {
		default:
			break;
	}
}
