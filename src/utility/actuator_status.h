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

#ifndef ACTUATOR_STATUS_H
#define ACTUATOR_STATUS_H

#include "size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { ACTUATOR_STATE_READY = 0, ACTUATOR_STATE_BUSY, ACTUATOR_STATE_ERROR } actuator_state_t;

typedef struct {
    char value[READING_SIZE];
    actuator_state_t state;
} actuator_status_t;

void actuator_status_init(actuator_status_t* actuator_status, char* value, actuator_state_t state);

char* actuator_status_get_value(actuator_status_t* actuator_status);

actuator_state_t actuator_status_get_state(actuator_status_t* actuator_status);

#ifdef __cplusplus
}
#endif

#endif
