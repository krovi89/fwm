#ifndef MESSAGES_H
#define MESSAGES_H

#define FWM_MESSAGE_TYPE_CONFIGURE 1

extern const unsigned char message_header[3];

void fwm_process_message(unsigned char *message, int length);

#endif
