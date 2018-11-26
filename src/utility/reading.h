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

#ifndef READING_H
#define READING_H

#include "actuator_status.h"
#include "manifest_item.h"
#include "size_definitions.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    manifest_item_t manifest_item;

    actuator_state_t actuator_state;

    char reading_data[READING_DIMENSIONS][READING_SIZE];

    uint32_t rtc;
} reading_t;

void reading_init(reading_t* reading, manifest_item_t* item);

void reading_clear(reading_t* reading);
void reading_clear_array(reading_t* first_reading, size_t readings_count);

void reading_set_data(reading_t* reading, char* data);
char* reading_get_data(reading_t* reading);

bool reading_get_delimited_data(reading_t* reading, char* buffer, size_t buffer_size);

void reading_set_data_at(reading_t* reading, char* data, size_t data_position);
char* reading_get_data_at(reading_t* reading, size_t data_position);

manifest_item_t* reading_get_manifest_item(reading_t* reading);

void reading_set_rtc(reading_t* reading, uint32_t rtc);
uint32_t reading_get_rtc(reading_t* reading);

void reading_set_actuator_state(reading_t* reading, actuator_state_t actuator_state);
actuator_state_t reading_get_actuator_state(reading_t* reading);

#ifdef __cplusplus
}
#endif

#endif
