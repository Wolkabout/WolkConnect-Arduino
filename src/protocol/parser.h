/*
 * Copyright 2024 WolkAbout Technology s.r.o.
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

#include "model/feed.h"
#include "model/utc_command.h"
#include "model/outbound_message.h"
#include "utility/wolk_utils.h"
#include "utility/wolk_types.h"
#include "protocol/json_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PARSER_TYPE
} parser_type_t;

typedef struct {
    parser_type_t type;
    bool is_initialized;

    char P2D_TOPIC[TOPIC_SIZE];
    char D2P_TOPIC[TOPIC_SIZE];
    char FEED_VALUES_MESSAGE_TOPIC[TOPIC_SIZE];
    char SYNC_TIME_TOPIC[TOPIC_SIZE];
    char ERROR_TOPIC[TOPIC_SIZE];

    size_t (*serialize_feeds)(feed_t* feeds, data_type_t type, size_t num_feeds, size_t feed_element_size,
                              char* buffer, size_t buffer_size);


    bool (*deserialize_time)(char* buffer, size_t buffer_size, utc_command_t* utc_command);

    bool (*create_topic)(const char direction[TOPIC_DIRECTION_SIZE], const char device_key[DEVICE_KEY_SIZE],
                         const char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE]);
    size_t (*deserialize_feeds_value_message)(char* buffer, size_t buffer_size, feed_t* feeds_received);

    bool (*serialize_sync_time)(const char* device_key, outbound_message_t* outbound_message);

    bool (*serialize_keep_alive_message)(const char* device_key, outbound_message_t* outbound_message);
} parser_t;

void initialize_parser(parser_t* parser, parser_type_t parser_type);

/**** Feed ****/
size_t parser_serialize_feeds(parser_t* parser, feed_t* feeds, data_type_t type, size_t num_feeds,
                              size_t feed_element_size, char* buffer, size_t buffer_size);
size_t parser_deserialize_feeds_message(parser_t* parser, char* buffer, size_t buffer_size, feed_t* feeds_received);
/**** Feed ****/

/**** Utility ****/
bool parser_create_topic(parser_t* parser, char direction[TOPIC_DIRECTION_SIZE], char device_key[DEVICE_KEY_SIZE],
                         char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE]);
bool parser_serialize_keep_alive_message(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);
bool parser_serialize_sync_time(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);
bool parser_deserialize_time(parser_t* parser, char* buffer, size_t buffer_size, utc_command_t* utc_command);
/**** Utility ****/
#ifdef __cplusplus
}
#endif

#endif
