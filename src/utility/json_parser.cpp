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

#include "json_parser.h"
#include "reading.h"
#include "jsmn.h"
#include "size_definitions.h"
#include "wolk_utils.h"
#include "actuator_status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


enum {
    /* Maximum number of characters in command name */
    COMMAND_MAX_SIZE = 10,

    /* Maximum number of characters in command argument */
    COMMAND_MAX_ARGUMENT_PART_SIZE = READING_DIMENSIONS,

    /* 5 + (2 * number of configuration items) */
    CONFIG_MESSAGE_MAX_JSON_TOKENS = 12
};

static bool all_readings_have_equal_rtc(reading_t* first_reading, size_t num_readings)
{
    size_t i;
    uint32_t rtc;

    reading_t* current_reading = first_reading;
    rtc = reading_get_rtc(current_reading);

    for (i = 0; i < num_readings; ++i, ++current_reading) {
        if (rtc != reading_get_rtc(current_reading)) {
            return false;
        }
    }

    return true;
}

static bool serialize_sensor(reading_t* reading, char* buffer, size_t buffer_size)
{
    char data_buffer[PARSER_INTERNAL_BUFFER_SIZE];
    if (!reading_get_delimited_data(reading, data_buffer, PARSER_INTERNAL_BUFFER_SIZE)) {
        return false;
    }

    if (reading_get_rtc(reading) > 0 &&
        snprintf(buffer, buffer_size, "{\"utc\":%u,\"data\":\"%s\"}",
                 reading_get_rtc(reading),
                 data_buffer) >= (int)buffer_size) {
            return false;
    } else if (reading_get_rtc(reading) == 0 &&
        snprintf(buffer, buffer_size, "{\"data\":\"%s\"}",
        data_buffer) >= (int)buffer_size) {
        return false;
    }

    return true;
}

static bool serialize_actuator(reading_t* reading, char* buffer, size_t buffer_size)
{
    char data_buffer[PARSER_INTERNAL_BUFFER_SIZE];
    if (!reading_get_delimited_data(reading, data_buffer, PARSER_INTERNAL_BUFFER_SIZE)) {
        return false;
    }

    switch (reading_get_actuator_state(reading)) {
    case ACTUATOR_STATE_READY:
        if (snprintf(buffer, buffer_size, "{\"status\":%s,\"value\":\"%s\"}",
                     "\"READY\"",
                     data_buffer) >= (int)buffer_size) {
            return false;
        }
        break;

    case ACTUATOR_STATE_BUSY:
        if (snprintf(buffer, buffer_size, "{\"status\":%s,\"value\":\"%s\"}",
                     "\"BUSY\"",
                     data_buffer) >= (int)buffer_size) {
            return false;
        }
        break;

    case ACTUATOR_STATE_ERROR:
        if (snprintf(buffer, buffer_size, "{\"status\":%s,\"value\":\"%s\"}",
                     "\"ERROR\"",
                     data_buffer) >= (int)buffer_size) {
            return false;
        }
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }

    return true;
}

static bool serialize_reading(reading_t* reading, char* buffer, size_t buffer_size)
{
    switch(manifest_item_get_reading_type(reading_get_manifest_item(reading))) {
    case READING_TYPE_SENSOR:
        return serialize_sensor(reading, buffer, buffer_size);

    case READING_TYPE_ACTUATOR:
        return serialize_actuator(reading, buffer, buffer_size);

    default:
        /* Sanity check*/
        WOLK_ASSERT(false);
    }

    return false;
}

static size_t serialize_readings(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size)
{
    WOLK_UNUSED(num_readings);

    return serialize_reading(first_reading, buffer, buffer_size) ? 1 : 0;
}

size_t json_serialize_readings(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(num_readings > 0);

    if (num_readings > 1 && all_readings_have_equal_rtc(first_reading, num_readings)) {
         return serialize_readings(first_reading, num_readings, buffer, buffer_size);
    } else {
        return serialize_reading(first_reading, buffer, buffer_size)  ? 1 : 0;
    }
}

static bool json_token_str_equal(const char *json, jsmntok_t *tok, const char *s)
{
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
            strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return true;
    }

    return false;
}

static bool deserialize_command(char* buffer, actuator_command_t* command)
{
    jsmn_parser parser;
    jsmntok_t tokens[10]; /* No more than 10 JSON token(s) are expected, check jsmn documentation for token definition */
    int i;
    int parser_result;

    char command_buffer[COMMAND_MAX_SIZE];
    char value_buffer[READING_SIZE];

    memset (command_buffer, 0, COMMAND_MAX_SIZE);
    memset (value_buffer, 0, READING_SIZE);

    jsmn_init(&parser);
    parser_result = jsmn_parse(&parser, buffer, strlen(buffer), tokens, WOLK_ARRAY_LENGTH(tokens));


    /* Received JSON must be valid, and top level element must be object*/
    if (parser_result < 1 || tokens[0].type != JSMN_OBJECT || parser_result >= (int) WOLK_ARRAY_LENGTH(tokens)) {
        return false;
    }



    for (i = 1; i < parser_result; i++) {
        if (json_token_str_equal(buffer, &tokens[i], "command")) {
            strncpy(command_buffer,  buffer + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);
            // if (snprintf(command_buffer, WOLK_ARRAY_LENGTH(command_buffer), "%.*s", tokens[i + 1].end - tokens[i + 1].start,
            //              buffer + tokens[i + 1].start) >= (int)WOLK_ARRAY_LENGTH(command_buffer)) {
            //     return false;
            // }

            i++;
        } else if (json_token_str_equal(buffer, &tokens[i], "value")) {
            // if (snprintf(value_buffer, WOLK_ARRAY_LENGTH(value_buffer), "%.*s", tokens[i + 1].end - tokens[i + 1].start,
            //              buffer + tokens[i + 1].start) >= (int)WOLK_ARRAY_LENGTH(value_buffer)) {
            //     return false;
            // }

            strncpy(value_buffer, buffer + tokens[i + 1].start, tokens[i + 1].end - tokens[i + 1].start);

            i++;

        } else {
            return false;
        }
    }

    if (strcmp(command_buffer, "STATUS") == 0) {
        actuator_command_init(command, ACTUATOR_COMMAND_TYPE_STATUS, "", "");
    } else if (strcmp(command_buffer, "SET") == 0) {
        actuator_command_init(command, ACTUATOR_COMMAND_TYPE_SET, "", value_buffer);
    } else {
        actuator_command_init(command, ACTUATOR_COMMAND_TYPE_UNKNOWN, "", value_buffer);
    }

    return true;
}

size_t json_deserialize_commands(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer_size);
    WOLK_UNUSED(commands_buffer_size);

    return deserialize_command(buffer, commands_buffer) ? 1 : 0;
}

size_t json_serialize_configuration_items(configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size)
{
    size_t i;
    size_t num_serialized_config_items = 0;
    size_t num_bytes_to_write;

    configuration_item_t* current_config_item = first_config_item;

    memset(buffer, '\0', buffer_size);

    if (snprintf(buffer, buffer_size, "{") >= (int)buffer_size) {
        return num_serialized_config_items;
    }

    for (i = 0; i < num_config_items; ++i) {
        char* conf_item_name = configuration_item_get_name(current_config_item);
        char* conf_item_value = configuration_item_get_value(current_config_item);

        /* -1 so we can have enough space left to append closing '}' */
        num_bytes_to_write = buffer_size - strlen(buffer);
        if (snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer) - 1, "\"%s\":\"%s\"", conf_item_name, conf_item_value) >= (int)num_bytes_to_write - 1) {
            break;
        }

        ++num_serialized_config_items;
        if (i >= num_config_items - 1) {
            continue;
        }

        ++current_config_item;

        conf_item_name = configuration_item_get_name(current_config_item);
        conf_item_value = configuration_item_get_value(current_config_item);

        /* +4 for '"', +1 for ':', +1 for ',' delimiter between configuration items, +1 for closing '}' => +7 */
        if (strlen(buffer) + strlen(conf_item_name) + strlen(conf_item_value) + 7 > buffer_size) {
            break;
        }

        num_bytes_to_write = buffer_size - strlen(buffer);
        if (snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), ",") >= (int)num_bytes_to_write) {
            break;
        }
    }

    num_bytes_to_write = buffer_size - strlen(buffer);
    if (snprintf(buffer + strlen(buffer), buffer_size - strlen(buffer), "}") >= (int)num_bytes_to_write) {
        return num_serialized_config_items;
    }

    return num_serialized_config_items;
}

size_t json_deserialize_configuration_items(char* buffer, size_t buffer_size, configuration_item_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer_size);

    jsmn_parser parser;
    jsmntok_t tokens[CONFIG_MESSAGE_MAX_JSON_TOKENS];
    int parser_result;

    configuration_item_command_t* current_config_command = commands_buffer;
    size_t num_deserialized_config_items = 0;
    int i;

    char command_buffer[COMMAND_MAX_SIZE];
    memset(commands_buffer, '\0', WOLK_ARRAY_LENGTH(command_buffer));

    char conf_item_name[CONFIGURATION_ITEM_NAME_SIZE];
    char conf_item_value[CONFIGURATION_ITEM_VALUE_SIZE];

    jsmn_init(&parser);
    parser_result = jsmn_parse(&parser, buffer, strlen(buffer), tokens, WOLK_ARRAY_LENGTH(tokens));

    /* Received JSON must be valid, and top level element must be object*/
    if (parser_result < 1 || tokens[0].type != JSMN_OBJECT) {
        return num_deserialized_config_items;
    }

    for (i = 1; i < parser_result && num_deserialized_config_items < commands_buffer_size; i++) {
        if (json_token_str_equal(buffer, &tokens[i], "command")) {
            if (snprintf(command_buffer, WOLK_ARRAY_LENGTH(command_buffer), "%.*s", tokens[i + 1].end - tokens[i + 1].start,
                         buffer + tokens[i + 1].start) >= (int)WOLK_ARRAY_LENGTH(command_buffer)) {
                continue;
            }

            if (strcmp(command_buffer, "CURRENT") == 0) {
                configuration_item_command_init(current_config_command, CONFIG_ITEM_COMMAND_TYPE_STATUS, "", "");
                ++num_deserialized_config_items;
                break;
            }

            i += 3;
        } else {
            if (strlen(command_buffer) == 0) {
                continue;
            }

            if (snprintf(conf_item_name, WOLK_ARRAY_LENGTH(conf_item_name), "%.*s", tokens[i].end - tokens[i].start,
                         buffer + tokens[i].start) >= (int)WOLK_ARRAY_LENGTH(conf_item_name)) {
                continue;
            }

            if (snprintf(conf_item_value, WOLK_ARRAY_LENGTH(conf_item_value), "%.*s", tokens[i + 1].end - tokens[i + 1].start,
                         buffer + tokens[i + 1].start) >= (int)WOLK_ARRAY_LENGTH(conf_item_value)) {
                continue;
            }
            ++i;

            if (strcmp(command_buffer, "SET") == 0) {
                configuration_item_command_init(current_config_command, CONFIG_ITEM_COMMAND_TYPE_SET, conf_item_name, conf_item_value);
            } else {
                configuration_item_command_init(current_config_command, CONFIG_ITEM_COMMAND_TYPE_UNKNOWN, "", "");
            }

            ++num_deserialized_config_items;
            ++current_config_command;
        }
    }

    return num_deserialized_config_items;
}
