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

#include "parser.h"
#include "actuator_command.h"
#include "configuration_item.h"
#include "json_parser.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void initialize_parser(parser_t* parser, parser_type_t parser_type)
{
    parser->type = parser_type;

    switch(parser->type)
    {
    case PARSER_TYPE_JSON:
        parser->serialize_readings = json_serialize_readings;
        parser->deserialize_commands = json_deserialize_commands;
        parser->deserialize_actuator_commands = json_deserialize_actuator_commands;

        parser->serialize_readings_topic = json_serialize_readings_topic;

        parser->serialize_configuration = json_serialize_configuration;
        parser->deserialize_configuration_commands = json_deserialize_configuration_command;

        parser->serialize_keep_alive_message = json_serialize_keep_alive_message;
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }
}

size_t parser_serialize_readings(parser_t* parser, reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size)
{
    return parser->serialize_readings(first_reading, num_readings, buffer, buffer_size);
}

size_t parser_deserialize_commands(parser_t* parser, char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer_size);
    return parser->deserialize_commands(buffer, buffer_size, commands_buffer, commands_buffer_size);
}

size_t parser_deserialize_actuator_commands(parser_t* parser, char* topic, size_t topic_size, char* buffer,
                                            size_t buffer_size, actuator_command_t* commands_buffer,
                                            size_t commands_buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(buffer_size < PAYLOAD_SIZE);
    WOLK_ASSERT(commands_buffer_size > 0);

    return parser->deserialize_actuator_commands(topic, topic_size, buffer, buffer_size, commands_buffer,
                                                 commands_buffer_size);
}

bool parser_serialize_readings_topic(parser_t* parser, const char* device_key, reading_t* first_reading,
                                     size_t num_readings, char* buffer, size_t buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(num_readings > 0);
    WOLK_ASSERT(buffer_size >= TOPIC_SIZE);

    return parser->serialize_readings_topic(first_reading, num_readings, device_key, buffer, buffer_size);
}

bool parser_serialize_configuration(parser_t* parser, const char* device_key,
                                    char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                    char (*value)[CONFIGURATION_VALUE_SIZE], size_t num_configuration_items,
                                    outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(num_configuration_items > 0);

    return parser->serialize_configuration(device_key, reference, value, num_configuration_items, outbound_message);
}

size_t parser_deserialize_configuration_commands(parser_t* parser, char* buffer, size_t buffer_size,
                                                 configuration_command_t* first_configuration_command,
                                                 size_t num_configuration_commands)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(buffer_size < PAYLOAD_SIZE);
    WOLK_ASSERT(num_configuration_commands > 0);

    return parser->deserialize_configuration_commands(buffer, buffer_size, first_configuration_command,
                                                      num_configuration_commands);
}

bool parser_serialize_keep_alive_message(parser_t* parser, const char* device_key, outbound_message_t* outbound_message)
{
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);

    return parser->serialize_keep_alive_message(device_key, outbound_message);
}

parser_type_t parser_get_type(parser_t* parser)
{
    /* Sanity check */
    WOLK_ASSERT(parser);

    return parser->type;
}

bool parser_is_initialized(parser_t* parser)
{
    /* Sanity check */
    WOLK_ASSERT(parser);

    return parser->is_initialized;
}

