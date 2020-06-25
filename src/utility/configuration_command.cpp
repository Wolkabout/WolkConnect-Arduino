/*
 * Copyright 2018 WolkAbout Technology s.r.o.
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

#include "configuration_command.h"
#include "size_definitions.h"
#include "wolk_utils.h"

#include <string.h>

void configuration_command_init(configuration_command_t* command, configuration_command_type_t type)
{
    /* Sanity check */
    WOLK_ASSERT(command);

    command->type = type;

    command->num_configuration_items = 0;
}

configuration_command_type_t configuration_command_get_type(configuration_command_t* command)
{
    return command->type;
}

void configuration_command_add(configuration_command_t* command, char* reference, char* value)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(reference) < CONFIGURATION_REFERENCE_SIZE);
    WOLK_ASSERT(strlen(value) < CONFIGURATION_VALUE_SIZE);

    strcpy(command->reference[command->num_configuration_items], reference);
    strcpy(command->value[command->num_configuration_items], value);

    command->num_configuration_items += 1;
}

size_t configuration_command_get_number_of_items(configuration_command_t* command)
{
    return command->num_configuration_items;
}

char (*configuration_command_get_references(configuration_command_t* command))[CONFIGURATION_REFERENCE_SIZE]
{
    return &command->reference[0];
}

char (*configuration_command_get_values(configuration_command_t* command))[CONFIGURATION_VALUE_SIZE]
{
    return &command->value[0];
}
