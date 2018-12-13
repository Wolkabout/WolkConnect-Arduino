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

#include "actuator_status.h"
#include "wolk_utils.h"

#include <string.h>

void actuator_status_init(actuator_status_t* actuator_status, char* value, actuator_state_t state)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(value) < READING_SIZE);

    strcpy(&actuator_status->value[0], value);
    actuator_status->state = state;
}

char* actuator_status_get_value(actuator_status_t* actuator_status)
{
    return actuator_status->value;
}

actuator_state_t actuator_status_get_state(actuator_status_t* actuator_status)
{
    return actuator_status->state;
}
