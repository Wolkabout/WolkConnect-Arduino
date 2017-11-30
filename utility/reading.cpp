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

#include "reading.h"
#include "manifest_item.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void reading_init(reading_t* reading, manifest_item_t* item)
{
    uint8_t i;
    uint8_t reading_dimensions = manifest_item_get_data_dimensions(item);

    memcpy(&reading->manifest_item, item, sizeof(reading->manifest_item));
    reading->actuator_status = ACTUATOR_STATUS_READY;
    reading->rtc = 0;

    for (i = 0; i < reading_dimensions; ++i) {
        reading_set_data(reading, "");
    }
}

void reading_clear(reading_t* reading)
{
    reading_init(reading, reading_get_manifest_item(reading));
}

void reading_clear_array(reading_t* first_reading, size_t readings_count)
{
    size_t i;

    reading_t* current_reading = first_reading;
    for (i = 0; i < readings_count; ++i, ++current_reading) {
        reading_clear(current_reading);
    }
}

void reading_set_data(reading_t* reading, char* data)
{
    reading_set_data_at(reading, data, 0);
}

char* reading_get_data(reading_t* reading)
{
    return reading_get_data_at(reading, 0);
}

bool reading_get_delimited_data(reading_t* reading, char* buffer, size_t buffer_size)
{
    size_t i;
    size_t data_dimensions = manifest_item_get_data_dimensions(reading_get_manifest_item(reading));
    char* data_delimiter = manifest_item_get_data_delimiter(reading_get_manifest_item(reading));

    memset(buffer, '\0', buffer_size);
    for (i = 0; i < data_dimensions; ++i) {
        if (i != 0) {
            size_t num_bytes_to_write = buffer_size - strlen(buffer);
            if (snprintf(buffer + strlen(buffer), (int)num_bytes_to_write, "%s", data_delimiter) >= (int)num_bytes_to_write) {
                return false;
            }
        }

        size_t num_bytes_to_write = buffer_size - strlen(buffer);
        if (snprintf(buffer + strlen(buffer), (int)num_bytes_to_write, "%s", reading_get_data_at(reading, i)) >= (int)num_bytes_to_write) {
            return false;
        }
    }

    return true;
}

void reading_set_data_at(reading_t* reading, char* data, size_t data_position)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(data) < READING_SIZE);
    WOLK_ASSERT(data_position < READING_DIMENSIONS);

    strcpy(reading->reading_data[data_position], data);
    reading->rtc = 0;
}

char* reading_get_data_at(reading_t* reading, size_t data_position)
{
    /* Sanity check */
    WOLK_ASSERT(data_position < reading->manifest_item->data_dimensions);

    return reading->reading_data[data_position];
}

manifest_item_t* reading_get_manifest_item(reading_t* reading)
{
    return &reading->manifest_item;
}

void reading_set_rtc(reading_t* reading, uint32_t rtc)
{
    reading->rtc = rtc;
}

uint32_t reading_get_rtc(reading_t* reading)
{
    return reading->rtc;
}

void reading_set_actuator_status(reading_t* reading, actuator_status_t actuator_status)
{
    /* Sanity check */
    WOLK_ASSERT(manifest_item_get_reading_type(reading_get_manifest_item(reading)) & READING_TYPE_ACTUATOR);

    reading->actuator_status = actuator_status;
}

actuator_status_t reading_get_actuator_status(reading_t* reading)
{
    /* Sanity check */
    WOLK_ASSERT(manifest_item_get_reading_type(reading_get_manifest_item(reading)) & READING_TYPE_ACTUATOR);

    return reading->actuator_status;
}
