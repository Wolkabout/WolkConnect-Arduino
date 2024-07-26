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

#ifndef PARSER_H
#define PARSER_H

#include "model/outbound_message.h"
#include "utility/wolk_types.h"
#include "model/feed.h"
#include "model/attribute.h"
#include "model/utc_command.h"
#include "model/parameter.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
    char ERROR_TOPIC[TOPIC_SIZE];

    size_t (*serialize_feeds)(feed_t* readings, data_type_t type, size_t num_readings, size_t reading_element_size,
                              char* buffer, size_t buffer_size);


    bool (*deserialize_time)(char* buffer, size_t buffer_size, utc_command_t* utc_command);
    bool (*deserialize_details_synchronization)(char* buffer, size_t buffer_size, feed_registration_t* feeds,
                                                size_t* number_of_feeds, attribute_t* attributes,
                                                size_t* number_of_attributes);

    bool (*create_topic)(const char direction[TOPIC_DIRECTION_SIZE], const char device_key[DEVICE_KEY_SIZE],
                         const char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE]);
    size_t (*deserialize_readings_value_message)(char* buffer, size_t buffer_size, feed_t* readings_received);

    bool (*serialize_feed_registration)(const char* device_key, feed_registration_t* feed, size_t number_of_feeds,
                                        outbound_message_t* outbound_message);
    bool (*serialize_feed_removal)(const char* device_key, feed_registration_t* feed, size_t number_of_feeds,
                                   outbound_message_t* outbound_message);
    bool (*serialize_pull_feed_values)(const char* device_key, outbound_message_t* outbound_message);
    bool (*serialize_sync_time)(const char* device_key, outbound_message_t* outbound_message);
    bool (*serialize_sync_details_synchronization)(const char* device_key, outbound_message_t* outbound_message);

    bool (*serialize_keep_alive_message)(const char* device_key, outbound_message_t* outbound_message);

    bool (*deserialize_pong)(char* buffer, size_t buffer_size, char* timestamp); 
} parser_t;

void initialize_parser(parser_t* parser, parser_type_t parser_type);

/**** Feed ****/
size_t parser_serialize_feeds(parser_t* parser, feed_t* readings, data_type_t type, size_t num_readings,
                              size_t reading_element_size, char* buffer, size_t buffer_size);
/**** Feed ****/

/**** Utility ****/
bool parser_deserialize_time(parser_t* parser, char* buffer, size_t buffer_size, utc_command_t* utc_command);
bool parser_deserialize_details_synchronization(parser_t* parser, char* buffer, size_t buffer_size,
                                                feed_registration_t* feeds, size_t* number_of_feeds,
                                                attribute_t* attributes, size_t* number_of_attributes);

bool parser_create_topic(parser_t* parser, char direction[TOPIC_DIRECTION_SIZE], char device_key[DEVICE_KEY_SIZE],
                         char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE]);

size_t parser_deserialize_feeds_message(parser_t* parser, char* buffer, size_t buffer_size, feed_t* readings_received);

size_t parser_deserialize_parameter_message(parser_t* parser, char* buffer, size_t buffer_size,
                                            parameter_t* parameter_message);

bool parser_serialize_feed_registration(parser_t* parser, const char* device_key, feed_registration_t* feed,
                                        size_t number_of_feeds, outbound_message_t* outbound_message);

bool parser_serialize_feed_removal(parser_t* parser, const char* device_key, feed_registration_t* feed,
                                   size_t number_of_feeds, outbound_message_t* outbound_message);
bool parser_serialize_pull_feed_values(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);

bool parser_serialize_pull_parameters(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);

bool parser_serialize_sync_parameters(parser_t* parser, const char* device_key, parameter_t* parameters,
                                      size_t number_of_parameters, outbound_message_t* outbound_message);

bool parser_serialize_sync_time(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);
bool parser_serialize_details_synchronization(parser_t* parser, const char* device_key,
                                              outbound_message_t* outbound_message);

bool parser_serialize_attribute(parser_t* parser, const char* device_key, attribute_t* attributes,
                                size_t number_of_attributes, outbound_message_t* outbound_message);

bool parser_serialize_parameter(parser_t* parser, const char* device_key, parameter_t* parameter,
                                size_t number_of_parameters, outbound_message_t* outbound_message);

bool parser_deserialize_pong(parser_t* parser, char* buffer, size_t buffer_size, char* timestamp);
/**** Utility ****/
#ifdef __cplusplus
}
#endif

#endif
