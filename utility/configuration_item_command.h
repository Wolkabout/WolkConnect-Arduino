#ifndef CONFIGURATION_ITEM_COMMAND_H
#define CONFIGURATION_ITEM_COMMAND_H

#include "size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CONFIG_ITEM_COMMAND_TYPE_STATUS = 0,
    CONFIG_ITEM_COMMAND_TYPE_SET,

    CONFIG_ITEM_COMMAND_TYPE_UNKNOWN
} configuration_command_type_t;

typedef struct {
    configuration_command_type_t type;

    char name[CONFIGURATION_ITEM_NAME_SIZE];
    char value[CONFIGURATION_ITEM_VALUE_SIZE];
} configuration_item_command_t;

void configuration_item_command_init(configuration_item_command_t* command, configuration_command_type_t type, char* name, char* value);

configuration_command_type_t configuration_item_command_get_type(configuration_item_command_t* command);

char* configuration_item_command_get_name(configuration_item_command_t* command);

char* configuration_item_command_get_value(configuration_item_command_t* command);

#ifdef __cplusplus
}
#endif

#endif
