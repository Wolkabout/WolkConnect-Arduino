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
