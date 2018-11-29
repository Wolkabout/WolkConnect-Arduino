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

#ifndef PARSER_H
#define PARSER_H

#include "actuator_command.h"
#include "configuration_item.h"
#include "configuration_command.h"
#include "reading.h"
#include "outbound_message.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PARSER_TYPE_MQTT = 0,
    PARSER_TYPE_JSON
} parser_type_t;

typedef struct {
    parser_type_t type;
    bool is_initialized;

    size_t (*serialize_readings)(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);
    //called deserialize actuator commands and a diff def
    size_t (*deserialize_commands)(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);
    size_t (*deserialize_actuator_commands)(char* topic, size_t topic_size, char* buffer, size_t buffer_size,
                                            actuator_command_t* commands_buffer, size_t commands_buffer_size);

    bool (*serialize_readings_topic)(reading_t* first_reading, size_t num_readings, const char* device_key,
                                     char* buffer, size_t buffer_size);

    size_t (*serialize_configuration)(const char* device_key, char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                      char (*value)[CONFIGURATION_VALUE_SIZE], size_t num_configuration_items,
                                      outbound_message_t* outbound_message);
    size_t (*deserialize_configuration_commands)(char* buffer, size_t buffer_size,
                                                 configuration_command_t* first_configuration_command,
                                                 size_t num_config_items);
    bool (*serialize_keep_alive_message)(const char* device_key, outbound_message_t* outbound_message);
} parser_t;

void initialize_parser(parser_t* parser, parser_type_t parser_type);

/**** Actuator command ****/
size_t parser_deserialize_actuator_commands(parser_t* parser, char* topic, size_t topic_size, char* buffer,
                                            size_t buffer_size, actuator_command_t* commands_buffer,
                                            size_t commands_buffer_size);
/**** Actuator command ****/

size_t parser_serialize_readings(parser_t* parser, reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);
bool parser_serialize_readings_topic(parser_t* parser, const char* device_key, reading_t* first_reading,
                                     size_t num_readings, char* buffer, size_t buffer_size);

size_t parser_deserialize_commands(parser_t* parser, char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);

/**** Configuration ****/
bool parser_serialize_configuration(parser_t* parser, const char* device_key,
                                    char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                    char (*value)[CONFIGURATION_VALUE_SIZE], size_t num_configuration_items,
                                    outbound_message_t* outbound_message);

size_t parser_deserialize_configuration_commands(parser_t* parser, char* buffer, size_t buffer_size,
                                                 configuration_command_t* first_configuration_command,
                                                 size_t num_configuration_commands);
/**** Configuration ****/
bool parser_serialize_keep_alive_message(parser_t* parser, const char* device_key,
                                         outbound_message_t* outbound_message);
parser_type_t parser_get_type(parser_t* parser);
bool parser_is_initialized(parser_t* parser);

#ifdef __cplusplus
}
#endif

#endif
