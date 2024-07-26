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

#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "model/attribute.h"
#include "model/feed.h"
#include "model/outbound_message.h"
#include "model/parameter.h"
#include "model/utc_command.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


extern const char JSON_P2D_TOPIC[TOPIC_DIRECTION_SIZE];
extern const char JSON_D2P_TOPIC[TOPIC_DIRECTION_SIZE];
extern const char JSON_FEED_REGISTRATION_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_FEED_REMOVAL_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_FEED_VALUES_MESSAGE_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_PULL_FEEDS_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_ATTRIBUTE_REGISTRATION_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_PARAMETERS_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_PULL_PARAMETERS_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_SYNC_PARAMETERS_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_SYNC_TIME_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_ERROR_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];
extern const char JSON_DETAILS_SYNCHRONIZATION_TOPIC[TOPIC_MESSAGE_TYPE_SIZE];

size_t json_serialize_feeds(feed_t* feeds, data_type_t type, size_t number_of_feeds, size_t feed_element_size,
                            char* buffer, size_t buffer_size);

size_t json_deserialize_feeds_value_message(char* buffer, size_t buffer_size, feed_t* feeds_received);

bool json_create_topic(const char direction[TOPIC_DIRECTION_SIZE], const char device_key[DEVICE_KEY_SIZE],
                       const char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE]);

bool json_serialize_feed_registration(const char* device_key, feed_registration_t* feed, size_t number_of_feeds,
                                      outbound_message_t* outbound_message);
bool json_serialize_feed_removal(const char* device_key, feed_registration_t* feed, size_t number_of_feeds,
                                 outbound_message_t* outbound_message);
bool json_serialize_pull_feed_values(const char* device_key, outbound_message_t* outbound_message);
bool json_serialize_parameter(const char* device_key, parameter_t* parameter, size_t number_of_parameters,
                              outbound_message_t* outbound_message);
bool json_serialize_pull_parameters(const char* device_key, outbound_message_t* outbound_message);
bool json_serialize_sync_parameters(const char* device_key, parameter_t* parameters, size_t number_of_parameters,
                                    outbound_message_t* outbound_message);
size_t json_deserialize_parameter_message(char* buffer, size_t buffer_size, parameter_t* parameter_message);

bool json_serialize_attribute(const char* device_key, attribute_t* attributes, size_t number_of_attributes,
                              outbound_message_t* outbound_message);
bool json_serialize_sync_time(const char* device_key, outbound_message_t* outbound_message);
bool json_serialize_sync_details_synchronization(const char* device_key, outbound_message_t* outbound_message);
bool json_deserialize_time(char* buffer, size_t buffer_size, utc_command_t* utc_command);
bool json_deserialize_details_synchronization(char* buffer, size_t buffer_size, feed_registration_t* feeds,
                                              size_t* number_of_feeds, attribute_t* attributes,
                                              size_t* number_of_attributes);

bool json_serialize_keep_alive_message(const char* device_key, outbound_message_t* outbound_message);
bool json_deserialize_pong(char* buffer, size_t buffer_size, char* timestamp);

#ifdef __cplusplus
}
#endif

#endif
