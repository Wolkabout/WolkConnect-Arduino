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

#include "protocol/parser.h"
#include "protocol/json_parser.h"
#include "utility/wolk_utils.h"

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
    case PARSER_TYPE:
        parser->is_initialized = true;
        parser->serialize_feeds = json_serialize_feeds;

        // parser->deserialize_commands = json_deserialize_commands;

        parser->create_topic = json_create_topic;

        parser->serialize_keep_alive_message = json_serialize_keep_alive_message;

        parser->deserialize_pong = json_deserialize_pong;

        strncpy(parser->P2D_TOPIC, JSON_P2D_TOPIC, TOPIC_MESSAGE_TYPE_SIZE);
        strncpy(parser->D2P_TOPIC, JSON_D2P_TOPIC, TOPIC_MESSAGE_TYPE_SIZE);
        strncpy(parser->FEED_VALUES_MESSAGE_TOPIC, JSON_FEED_VALUES_MESSAGE_TOPIC, TOPIC_MESSAGE_TYPE_SIZE);
        strncpy(parser->ERROR_TOPIC, JSON_ERROR_TOPIC, TOPIC_MESSAGE_TYPE_SIZE);
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }
}

size_t parser_serialize_feeds(parser_t* parser, feed_t* readings, data_type_t type, size_t num_readings,
                              size_t reading_element_size, char* buffer, size_t buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(num_readings > 0);
    WOLK_ASSERT(buffer_size >= PAYLOAD_SIZE);

    return parser->serialize_feeds(readings, type, num_readings, reading_element_size, buffer, buffer_size);
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

bool parser_deserialize_pong(parser_t* parser, char* buffer, size_t buffer_size, char* timestamp)
{
    WOLK_ASSERT(parser);
    WOLK_ASSERT(buffer_size < PAYLOAD_SIZE);

    return parser->deserialize_pong(buffer, buffer_size, timestamp);

}
