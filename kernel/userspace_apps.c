#include "string.h"
#include "userspace_apps.h"

#include <stdio.h>
struct uapps * get_uapp_by_name(const char * name)
{
    struct uapps * current_app;
    current_app = (struct uapps *)symbols_table;

    while (current_app->name) {
        if (!strcmp(current_app->name, name))
            return current_app;
        current_app++;
    }

    return NULL;
}