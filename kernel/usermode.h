#ifndef __USERMODE_H__
#define __USERMODE_H__

/**
 * Switch from kernel mode to the first
 * process in user mode.
 */
void goto_user_mode(uint32_t *old, uint32_t *new);

#endif