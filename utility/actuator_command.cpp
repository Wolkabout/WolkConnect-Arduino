/*
 * Copyright 2017 WolkAbout Technology s.r.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
