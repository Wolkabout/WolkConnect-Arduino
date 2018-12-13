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

#ifndef CONFIGURATION_COMMAND_H
#define CONFIGURATION_COMMAND_H

#include "size_definitions.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CONFIGURATION_COMMAND_TYPE_CURRENT = 0,
    CONFIGURATION_COMMAND_TYPE_SET,

    CONFIGURATION_COMMAND_TYPE_UNKNOWN
} configuration_command_type_t;

typedef struct {
    configuration_command_type_t type;

    char reference[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_REFERENCE_SIZE];
    char value[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_VALUE_SIZE];

    size_t num_configuration_items;
} configuration_command_t;

void configuration_command_init(configuration_command_t* command, configuration_command_type_t type);

configuration_command_type_t configuration_command_get_type(configuration_command_t* command);

void configuration_command_add(configuration_command_t* command, char* reference, char* value);

size_t configuration_command_get_number_of_items(configuration_command_t* command);

char (*configuration_command_get_references(configuration_command_t* command))[CONFIGURATION_REFERENCE_SIZE];

char (*configuration_command_get_values(configuration_command_t* command))[CONFIGURATION_VALUE_SIZE];

#ifdef __cplusplus
}
#endif

#endif
