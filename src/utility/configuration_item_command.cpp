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

#include "configuration_item_command.h"
#include "wolk_utils.h"

#include <string.h>

void configuration_item_command_init(configuration_item_command_t* command, configuration_command_type_t type, char* name, char* value)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(name) <= CONFIGURATION_ITEM_NAME_SIZE);
    WOLK_ASSERT(strlen(value) <= CONFIGURATION_ITEM_VALUE_SIZE);

    command->type = type;

    strcpy(command->name, name);
    strcpy(command->value, value);
}

configuration_command_type_t configuration_item_command_get_type(configuration_item_command_t* command)
{
    return command->type;
}

char* configuration_item_command_get_name(configuration_item_command_t* command)
{
    return command->name;
}

char* configuration_item_command_get_value(configuration_item_command_t* command)
{
    return command->value;
}
