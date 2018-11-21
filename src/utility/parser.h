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
#include "configuration_item_command.h"
#include "reading.h"

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

    size_t (*serialize_readings)(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);
    size_t (*deserialize_commands)(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);

    size_t (*serialize_configuration_items)(configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size);
    size_t (*deserialize_configuration_items)(char* buffer, size_t buffer_size, configuration_item_command_t* first_config_item, size_t num_config_items);
} parser_t;

void initialize_parser(parser_t* parser, parser_type_t parser_type);

size_t parser_serialize_readings(parser_t* parser, reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);
size_t parser_deserialize_commands(parser_t* parser, char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);

size_t parser_serialize_configuration_items(parser_t* parser, configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size);
size_t parser_deserialize_configuration_items(parser_t* parser, char* buffer, size_t buffer_size, configuration_item_command_t* first_config_item, size_t num_config_items);

#ifdef __cplusplus
}
#endif

#endif
