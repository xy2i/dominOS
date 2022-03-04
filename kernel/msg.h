#ifndef __MSG__H__
#define __MSG__H__

#include "queue.h"

#define NBQUEUE 20

struct mqueue {
	struct msg *head; /* First message */
	struct msg *tail; /* Last message */
	unsigned int size; /* Max number of messages */
	unsigned int count; /* Number of messages */
	struct list_link waiting_senders;
	struct list_link waiting_receivers;
};

struct msg {
	struct msg *next;
	int data;
};

// Crée une file de messages
int pcreate(int count);

// Détruit une file de messages
int pdelete(int id);

// Dépose un message dans une file
int psend(int id, int msg);

// Retire un message d'une file
int preceive(int id,int *msg);

// Réinitialise une file
int preset(int id);

// Renvoie l'état courant d'une file
int pcount(int id, int *count);

#endif