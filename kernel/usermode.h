#ifndef __USERMODE_H__
#define __USERMODE_H__

void goto_user_mode(int (*func_ptr)(void *), unsigned long esp);

#endif