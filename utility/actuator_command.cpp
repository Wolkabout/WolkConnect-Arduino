#include "actuator_command.h"
#include "wolk_utils.h"

#include <string.h>

void actuator_command_init(actuator_command_t* command, actuator_command_type_t type, char* reference, char* argument)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(reference) <= MANIFEST_ITEM_REFERENCE_SIZE);
    WOLK_ASSERT(strlen(argument) <= COMMAND_ARGUMENT_SIZE);

    memset(command->argument, 0, COMMAND_ARGUMENT_SIZE);

    command->type = type;

    strcpy(command->reference, reference);
    strcpy(command->argument, argument);
}

actuator_command_type_t actuator_command_get_type(actuator_command_t* command)
{
    return command->type;
}

char* actuator_command_get_reference(actuator_command_t* command)
{
    return command->reference;
}

void actuator_command_set_reference(actuator_command_t* command, char* reference)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(reference) < MANIFEST_ITEM_REFERENCE_SIZE);

    strcpy(command->reference, reference);
}

char* actuator_command_get_value(actuator_command_t* command)
{
    return command->argument;
}
