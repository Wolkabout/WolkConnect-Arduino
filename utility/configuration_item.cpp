#include "configuration_item.h"
#include "wolk_utils.h"

#include <stddef.h>
#include <stdbool.h>
#include <string.h>

void configuration_item_init(configuration_item_t* configuration_item, char* name, char* value)
{
    strcpy(configuration_item->name, name);
    strcpy(configuration_item->value, value);
}

char* configuration_item_get_name(configuration_item_t* configuration_item)
{
    return configuration_item->name;
}

char* configuration_item_get_value(configuration_item_t* configuration_item)
{
    return configuration_item->value;
}

void configuration_item_set_value(configuration_item_t* configuration_item, char* buffer)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(buffer) < CONFIGURATION_ITEM_VALUE_SIZE);

    strcpy(configuration_item->value, buffer);
}
