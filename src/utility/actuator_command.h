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
