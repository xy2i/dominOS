#ifndef __MSG__
#define __MSG__

#include "queue.h"

#define NBQUEUE 20

struct mqueue {
	struct msg *head; /* First message */
	unsigned int size; /* Max number of messages */
	unsigned int count; /* Number of messages */
	struct list_link waiting_senders;
	struct list_link waiting_receivers;
};

struct msg {
	struct msg *next;
	int data;
};

#endif