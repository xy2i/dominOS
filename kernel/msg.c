#include "msg.h"
#include "task.h"
#include "mem.h"

#define __MQUEUE_UNUSED 0

static struct mqueue *mqueues[NBQUEUE] = { __MQUEUE_UNUSED };

#define GET_MQUEUE_PTR(id) (mqueues[id])
#define MQUEUE_USED(id) (GET_MQUEUE_PTR(id) != MQUEUE_UNUSED)
#define MQUEUE_UNUSED(id) (GET_MQUEUE_PTR(id) == __MQUEUE_UNUSED)
#define MQUEUE_EMPTY(id) (GET_MQUEUE_PTR(id)->count == 0)
#define MQUEUE_FULL(id) (GET_MQUEUE_PTR(id)->count == GET_MQUEUE_PTR(id)->size)

static int first_available_queue(void)
{
	int mqueue_id;
	for (mqueue_id = 0; mqueue_id < NBQUEUE; mqueue_id++) {
		if (MQUEUE_UNUSED(mqueue_id))
			return mqueue_id;
	}
	return mqueue_id;
}

static void alloc_mqueue(int mqueue_id, int count)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(mqueue_id);
	mqueue_ptr = (struct mqueue *)mem_alloc(sizeof(struct mqueue));
	mqueue_ptr->head = NULL;
	mqueue_ptr->size = count;
	mqueue_ptr->count = 0;
	INIT_LIST_HEAD(&mqueue_ptr->waiting_senders);
	INIT_LIST_HEAD(&mqueue_ptr->waiting_receivers);
}

int pcreate(int count)
{
	int mqueue_id;
	if (count <= 0)
		return -2;
	if ((mqueue_id = first_available_queue()) == -1)
		return -1;
	alloc_mqueue(mqueue_id, count);
	return mqueue_id;
}

static void __add_msg(int id, int msg)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(id);
	struct msg * msg_ptr = mem_alloc(sizeof(struct msg));
	msg_ptr->data = msg;
	msg_ptr->next = mqueue_ptr->head;
	mqueue_ptr->head = msg_ptr;
	mqueue_ptr->count++;
}

int psend(int id, int msg)
{
	if (MQUEUE_UNUSED(id))
		return -1;
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);

	if (MQUEUE_EMPTY(id) && last != NULL) {
		__add_msg(id, msg);
		add_ready_task(last);
		schedule();
		return 0;
	}

	while (MQUEUE_FULL(id)) {
		schedule();
	}

	__add_msg(id, msg);
	return 0;
}


int preceive(int id, int * message)
{
	
	if (message == NULL)
}
