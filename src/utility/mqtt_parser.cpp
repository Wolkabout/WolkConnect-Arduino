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

#include "mqtt_parser.h"
#include "size_definitions.h"
#include "manifest_item.h"
#include "reading.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>



enum {
    /* Maximum number of characters in command name */
    COMMAND_MAX_SIZE = 10,

    /* Maximum number of characters in command argument */
    COMMAND_MAX_ARGUMENT_PART_SIZE = MANIFEST_ITEM_REFERENCE_SIZE + 1 + READING_DIMENSIONS /* +1 for ':' delimiter */
};

static bool append_to_buffer(char* buffer, size_t buffer_size, char* apendee)
{
    if (strlen(buffer) + strlen(apendee) > buffer_size) {
        return false;
    }

    strcpy(buffer + strlen(buffer), apendee);
    return true;
}

static bool append_actuator_status(char* buffer, size_t buffer_size, actuator_state_t actuator_state)
{
    char reading_buffer[PARSER_INTERNAL_BUFFER_SIZE];

    switch (actuator_state) {
    case ACTUATOR_STATE_READY:
        sprintf(reading_buffer, ":READY");
        break;

    case ACTUATOR_STATE_BUSY:
        sprintf(reading_buffer, ":BUSY");
        break;

    case ACTUATOR_STATE_ERROR:
        sprintf(reading_buffer, ":ERROR");
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }

    return append_to_buffer(buffer, buffer_size, reading_buffer);
}

static bool append_reading_prefix(reading_t* reading, char* buffer, size_t buffer_size, bool is_first_reading)
{
    if (manifest_item_get_reading_type(reading_get_manifest_item(reading)) == READING_TYPE_ACTUATOR) {
        if (snprintf(buffer, buffer_size, "STATUS ") >= (int)buffer_size) {
            return false;
        }
    } else if (manifest_item_get_reading_type(reading_get_manifest_item(reading)) == READING_TYPE_SENSOR && is_first_reading) {
        if (snprintf(buffer, buffer_size, "READINGS R:%u,", reading_get_rtc(reading)) >= (int)buffer_size) {
            return false;
        }
    } else if (manifest_item_get_reading_type(reading_get_manifest_item(reading)) == READING_TYPE_SENSOR && !is_first_reading) {
        if (snprintf(buffer, buffer_size, "R:%u,", reading_get_rtc(reading)) >= (int)buffer_size) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}

static bool append_reading(reading_t* reading, char* buffer, size_t buffer_size)
{
    if (snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "%s:%s",
                 manifest_item_get_reference(reading_get_manifest_item(reading)),
                 reading_get_data_at(reading, 0)) >= (int)buffer_size) {
        return false;
    }

    return true;
}

static bool serialize_reading(reading_t* reading, char* buffer, size_t buffer_size, bool is_first_reading)
{
    const size_t reading_buffer_size = PARSER_INTERNAL_BUFFER_SIZE;
    char reading_buffer[PARSER_INTERNAL_BUFFER_SIZE];

    uint8_t i;
    manifest_item_t* manifest_item = reading_get_manifest_item(reading);

    if (!append_reading_prefix(reading, reading_buffer, reading_buffer_size, is_first_reading)) {
        return false;
    }

    if (!append_reading(reading, reading_buffer, reading_buffer_size)) {
        return false;
    }

    const uint8_t data_size = manifest_item_get_data_dimensions(manifest_item);
    if (data_size == 1) {
        if ((manifest_item_get_reading_type(reading_get_manifest_item(reading)) == READING_TYPE_ACTUATOR) &&
             !append_actuator_status(reading_buffer, reading_buffer_size, reading_get_actuator_state(reading))) {
            return false;
        }

        return append_to_buffer(buffer, buffer_size, reading_buffer);
    }

    char* delimiter = manifest_item_get_data_delimiter(manifest_item);
    for (i = 1; i < manifest_item_get_data_dimensions(manifest_item); ++i) {
        size_t num_bytes_to_write = reading_buffer_size - strlen(reading_buffer);
        if (snprintf(reading_buffer + strlen(reading_buffer), num_bytes_to_write,
                     "%s%s",
                     delimiter,
                     reading_get_data_at(reading, i)) >= (int)num_bytes_to_write) {
            return false;
        }
    }

    if ((manifest_item_get_reading_type(reading_get_manifest_item(reading)) == READING_TYPE_ACTUATOR) &&
         !append_actuator_status(reading_buffer, reading_buffer_size, reading_get_actuator_state(reading))) {
        return false;
    }

    return append_to_buffer(buffer, buffer_size, reading_buffer);
}

static bool serialize_readings_delimiter(char* buffer, size_t buffer_size, char* delimiter)
{
    char delimiter_buffer[MANIFEST_ITEM_DATA_DELIMITER_SIZE + 1];
    if(snprintf(delimiter_buffer, (int)WOLK_ARRAY_LENGTH(delimiter_buffer), "%s", delimiter) >= (int)WOLK_ARRAY_LENGTH(delimiter_buffer)) {
        return false;
    }

    return append_to_buffer(buffer, buffer_size, delimiter_buffer);
}

static bool serialize_readings_ending(char* buffer, size_t buffer_size)
{
    char delimiter_buffer[MANIFEST_ITEM_DATA_DELIMITER_SIZE + 1];

    if(snprintf(delimiter_buffer, (int)WOLK_ARRAY_LENGTH(delimiter_buffer), ";") >= (int)WOLK_ARRAY_LENGTH(delimiter_buffer)) {
        return false;
    }

    return append_to_buffer(buffer, buffer_size, delimiter_buffer);
}

static bool deserialize_command(char* buffer, actuator_command_t* command)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(buffer) <= COMMAND_MAX_SIZE + COMMAND_MAX_ARGUMENT_PART_SIZE + 1);

    /*char type_part[COMMAND_MAX_SIZE];
    char argument_part[COMMAND_MAX_ARGUMENT_PART_SIZE];

    if (sscanf(buffer, "%s %s", type_part, argument_part) != 2) {
        return false;
    }

    if (strcmp(type_part, "STATUS") == 0) {
        actuator_command_init(command, ACTUATOR_COMMAND_TYPE_STATUS, argument_part, "");
        return true;
    } else if (strcmp(type_part, "SET") == 0){
        char reference[MANIFEST_ITEM_REFERENCE_SIZE];
        char argument[COMMAND_ARGUMENT_SIZE];

        if (sscanf(argument_part, "%[^:]:%s", reference, argument) != 2) {
            return false;
        }

        actuator_command_init(command, ACTUATOR_COMMAND_TYPE_SET, reference, argument);
        return true;
    }*/

    //ToDo fix

    return false;
}

size_t mqtt_serialize_readings(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(num_readings > 0);

    size_t num_serialized_readings;
    reading_t* current_reading = first_reading;
    reading_t* previous_reading = current_reading;
    for(num_serialized_readings = 0; num_serialized_readings < num_readings; ++num_serialized_readings, ++current_reading) {
        bool is_first_reading = num_serialized_readings == 0;

        if (num_serialized_readings != 0) {
            switch(manifest_item_get_reading_type(reading_get_manifest_item(current_reading)))
            {
            case READING_TYPE_SENSOR:
                if (manifest_item_get_reading_type(reading_get_manifest_item(previous_reading)) == READING_TYPE_SENSOR) {
                    serialize_readings_delimiter(buffer, buffer_size, "|");
                } else {
                    serialize_readings_delimiter(buffer, buffer_size, ";");
                    is_first_reading = true;
                }
                break;

            case READING_TYPE_ACTUATOR:
                serialize_readings_delimiter(buffer, buffer_size, ";");
                break;

            default:
                serialize_readings_delimiter(buffer, buffer_size, ";");
                break;
            }
        }

        if(!serialize_reading(current_reading, buffer, buffer_size, is_first_reading)) {
            break;
        }

        previous_reading = current_reading;
    }

    serialize_readings_ending(buffer, buffer_size);

    return num_serialized_readings;
}

size_t mqtt_deserialize_commands(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer_size);

    /* Sanity check */
    WOLK_ASSERT(PARSER_INTERNAL_BUFFER_SIZE >= buffer_size);

    size_t num_serialized_commands = 0;
    actuator_command_t* current_command = commands_buffer;
    char tmp_buffer[PARSER_INTERNAL_BUFFER_SIZE];
    strcpy(tmp_buffer, buffer);

    char* p = strtok (tmp_buffer, ";");
    while (p != NULL) {
        if (num_serialized_commands == commands_buffer_size) {
            break;
        }

        if (deserialize_command(p, current_command)) {
            ++num_serialized_commands;
            ++current_command;
        }

        p = strtok(NULL, ";");
    }

    return num_serialized_commands;
}

size_t mqtt_serialize_configuration_items(configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size)
{
    WOLK_UNUSED(first_config_item);
    WOLK_UNUSED(num_config_items);
    WOLK_UNUSED(buffer);
    WOLK_UNUSED(buffer_size);

    /* Sanity check */
    WOLK_ASSERT(false);

    return 0;
}

size_t mqtt_deserialize_configuration_items(char* buffer, size_t buffer_size, configuration_item_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer);
    WOLK_UNUSED(buffer_size);
    WOLK_UNUSED(commands_buffer);
    WOLK_UNUSED(commands_buffer_size);

    /* Sanity check */
    WOLK_ASSERT(false);

    return 0;
}
