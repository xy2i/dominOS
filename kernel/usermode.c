#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "parameters.h"
#include "start.h"
#include "task.h"
#include "memory.h"
#include "userspace_apps.h"

int user_start(void *);
void goto_user_mode(int (*func_ptr)(void *), unsigned long esp);

int start_init_process(void *arg __attribute__((unused)))
{
    struct vm_area *vm_area;
    struct uapps *init_app;
    uint32_t init_app_size;

    init_app = get_uapp_by_name("init");

    if (!init_app)
	panic("No init process found!\n");

    init_app_size = init_app->end - init_app->start;

    vm_area = alloc_vm_area((uint32_t)user_start,
			    ((uint32_t)user_start) + init_app_size,
			    1, // R/W
			    1); // User accessible

    add_vm_area(current()->mm, vm_area);
    map_vm_area(current()->mm, vm_area);

    memcpy(user_start, init_app->start, init_app_size);

    goto_user_mode(user_start, USTACK_START - 8);

    return 0;
}

void first_user_task(void)
{
    start(start_init_process, 512, 128, "init", NULL);
}
