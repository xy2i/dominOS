#include "msg.h"
#include "task.h"
#include "mem.h"

#define __MQUEUE_UNUSED 0

static struct mqueue *mqueues[NBQUEUE] = { __MQUEUE_UNUSED };

static int cpt_rst = 0;

#define GET_MQUEUE_PTR(id) (mqueues[id])
#define SET_MQUEUE_PTR(id, ptr) (mqueues[id] = ptr)
#define MQUEUE_USED(id) (GET_MQUEUE_PTR(id) != __MQUEUE_UNUSED)
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
	struct mqueue * mqueue_ptr = (struct mqueue *)mem_alloc(sizeof(struct mqueue));
	mqueue_ptr->head = NULL;
	mqueue_ptr->size = count;
	mqueue_ptr->count = 0;
	INIT_LIST_HEAD(&mqueue_ptr->waiting_senders);
	INIT_LIST_HEAD(&mqueue_ptr->waiting_receivers);
	SET_MQUEUE_PTR(mqueue_id, mqueue_ptr);
}

static void free_mqueue(int mqueue_id)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(mqueue_id);
	if(!MQUEUE_EMPTY(mqueue_id)){
		struct msg *msg_ptr = mqueue_ptr->head;
		struct msg *next = msg_ptr->next;
		while(next != NULL){
			next = msg_ptr->next;
			mem_free(msg_ptr, sizeof(struct msg *));
			msg_ptr = next;
		}
		mem_free(msg_ptr, sizeof(struct msg *));
	}
	mem_free(mqueue_ptr, sizeof(struct mqueue));
	GET_MQUEUE_PTR(mqueue_id) = __MQUEUE_UNUSED;
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

	if(mqueue_ptr->count == 0){
		mqueue_ptr->head = msg_ptr;
		mqueue_ptr->tail = msg_ptr;
	}else{
		mqueue_ptr->tail->next = msg_ptr;
		mqueue_ptr->tail = msg_ptr;
	}

	mqueue_ptr->count++;
}

static int __pop_msg(int id)
{
	struct mqueue * mqueue_ptr = GET_MQUEUE_PTR(id);
	mqueue_ptr->count--;

	int msg = mqueue_ptr->head->data;
	mqueue_ptr->head = mqueue_ptr->head->next;

	return msg;
}

int psend(int id, int msg)
{	
	int rst = cpt_rst; 

	if (MQUEUE_UNUSED(id))
		return -1;
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);

	// On réveille un processus en attente sur la lecture
	if (MQUEUE_EMPTY(id) && last != NULL) {
		__add_msg(id, msg);

		set_task_ready_or_running(last);

		return 0;
	}

	while (MQUEUE_FULL(id)) {
		queue_add(current(),&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks, priority); // On ajoute la task aux ws
		
		set_task_interrupt_msg(current());
	}

	// Test pdelete et preset
	if (MQUEUE_UNUSED(id)||(rst<cpt_rst))
		return -1;

	__add_msg(id, msg);
	return 0;
}

int preceive(int id, int *message)
{	
	int rst = cpt_rst; 

	if (MQUEUE_UNUSED(id))
		return -1;
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks);

	// On réveille un processus en attente sur l'écriture
	if (MQUEUE_FULL(id) && last != NULL) {
	    int msg = __pop_msg(id);
	    if (message != NULL)
		*message = msg;

	    set_task_ready_or_running(last);

	    return 0;
	}

	while (MQUEUE_EMPTY(id)) {
		queue_add(current(),&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks, priority); // On ajoute la task aux wr

		set_task_interrupt_msg(current());
	}

	// Test pdelete et preset
	if (MQUEUE_UNUSED(id)||(rst<cpt_rst))
		return -1;

	int msg = __pop_msg(id);
	if (message != NULL)
		*message = msg;
	return 0;
}

int pdelete(int id)
{
	if (MQUEUE_UNUSED(id))
		return -1;

	cpt_rst++;

	// Il faut débloquer les processus en attente avec une valeur négative
	struct task * last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task, tasks);
	while(last != NULL){
	    set_task_ready_or_running(last);
	    last = queue_out(&GET_MQUEUE_PTR(id)->waiting_senders, struct task,
			     tasks);
	}
	last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers, struct task, tasks);
	while(last != NULL){
	    set_task_ready_or_running(last);
	    last = queue_out(&GET_MQUEUE_PTR(id)->waiting_receivers,
			     struct task, tasks);
	}

    // Liberer les ressources
	free_mqueue(id);

	return 0;
}

int pcount(int id, int *count)
{
    if (MQUEUE_UNUSED(id))
	return -1; // invalid fid value

    if (count == NULL)
	return 0; // count is NULL

    struct task *p;
    if (GET_MQUEUE_PTR(id)->count == 0) {
	int count_wr = 0;
	queue_for_each(p, &GET_MQUEUE_PTR(id)->waiting_receivers, struct task,
		       tasks)
	{
	    count_wr++;
	}
	*count = -1 * count_wr;
    } else {
	int count_ws = 0;
	queue_for_each(p, &GET_MQUEUE_PTR(id)->waiting_senders, struct task,
		       tasks)
	{
	    count_ws++;
	}
	*count = count_ws + GET_MQUEUE_PTR(id)->count;
    }
    return 0;
}

int preset(int id){
	if (MQUEUE_UNUSED(id))
		return -1;

	cpt_rst++;

	int count = GET_MQUEUE_PTR(id)->count;
	pdelete(id);
	alloc_mqueue(id, count);

	return 0;
}

//Fonction appellée par chprio si on ne trouve pas la task dans les listes de task.c
int update_position_mqueue(int pid, int priority)
{
	for(int i=0; i<NBQUEUE; i++){
		if(MQUEUE_USED(i)){
			struct task *current = NULL;

			queue_for_each(current, &GET_MQUEUE_PTR(i)->waiting_senders, struct task, tasks){
				if (current->pid == pid) {
					int former_priority = current->priority;
					current->priority = priority;
					queue_del(current, tasks);
					queue_add(current, &GET_MQUEUE_PTR(i)->waiting_senders, struct task, tasks, priority);
					return former_priority;
				}
			}

			queue_for_each(current, &GET_MQUEUE_PTR(i)->waiting_receivers, struct task, tasks){
				if (current->pid == pid) {
					int former_priority = current->priority;
					current->priority = priority;
					queue_del(current, tasks);
					queue_add(current, &GET_MQUEUE_PTR(i)->waiting_receivers, struct task, tasks, priority);
					return former_priority;
				}
			}

		}
	}
	return -1;
}