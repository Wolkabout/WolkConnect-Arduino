#ifndef CONFIGURATION_ITEM_H
#define CONFIGURATION_ITEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "size_definitions.h"

#include <stddef.h>
#include <stdbool.h>

typedef struct {
    char name[CONFIGURATION_ITEM_NAME_SIZE];
    char value[CONFIGURATION_ITEM_VALUE_SIZE];
} configuration_item_t;

void configuration_item_init(configuration_item_t* configuration_item, char* name, char* value);

char* configuration_item_get_name(configuration_item_t* configuration_item);

char* configuration_item_get_value(configuration_item_t* configuration_item);
void configuration_item_set_value(configuration_item_t* configuration_item, char* buffer);

#ifdef __cplusplus
}
#endif

#endif
