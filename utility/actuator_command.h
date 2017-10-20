#ifndef ACTUATOR_COMMAND_H
#define ACTUATOR_COMMAND_H

#include "size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ACTUATOR_COMMAND_TYPE_STATUS = 0,
    ACTUATOR_COMMAND_TYPE_SET,

    ACTUATOR_COMMAND_TYPE_UNKNOWN
} actuator_command_type_t;

typedef struct {
    actuator_command_type_t type;

    char reference[MANIFEST_ITEM_REFERENCE_SIZE];
    char argument[COMMAND_ARGUMENT_SIZE];
} actuator_command_t;

void actuator_command_init(actuator_command_t* command, actuator_command_type_t type, char* reference, char* argument);

actuator_command_type_t actuator_command_get_type(actuator_command_t* command);

char* actuator_command_get_reference(actuator_command_t* command);
void actuator_command_set_reference(actuator_command_t* command, char* reference);

char* actuator_command_get_value(actuator_command_t* command);

#ifdef __cplusplus
}
#endif

#endif
