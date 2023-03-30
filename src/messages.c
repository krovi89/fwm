#include <stdlib.h>

#include "messages.h"
#include "fwm.h"
#include "log.h"

const unsigned char message_header[3] = { 'f', 'w', 'm' };

void fwm_process_message(unsigned char *message, int length) {
	message += sizeof message_header;
	unsigned char message_type = *message++;

	switch (message_type) {
		case FWM_MESSAGE_TYPE_EXIT:
			fwm_log_info("Received FWM_MESSAGE_TYPE_EXIT, exiting..");
			fwm_exit(EXIT_SUCCESS);
		default:
			break;
	}
}
