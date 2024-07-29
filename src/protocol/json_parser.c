/*
 * Copyright 2022 WolkAbout Technology s.r.o.
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

#include "protocol/json_parser.h"
#include "utility/size_definitions.h"
#include "utility/jsmn.h"
#include "utility/wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JSON_TOKEN_SIZE FEEDS_MAX_NUMBER
#define UTC_MILLISECONDS_LENGTH 14

#define JSON_ARRAY_LEFT_BRACKET "["
#define JSON_ARRAY_RIGHT_BRACKET "]"
#define JSON_ELEMENTS_SEPARATORS "[,]\""

const char JSON_P2D_TOPIC[TOPIC_DIRECTION_SIZE] = "p2d";
const char JSON_D2P_TOPIC[TOPIC_DIRECTION_SIZE] = "d2p";
const char JSON_FEED_VALUES_MESSAGE_TOPIC[TOPIC_MESSAGE_TYPE_SIZE] = "feed_values";
const char JSON_SYNC_TIME_TOPIC[TOPIC_MESSAGE_TYPE_SIZE] = "time";
const char JSON_ERROR_TOPIC[TOPIC_MESSAGE_TYPE_SIZE] = "error";

static bool json_token_str_equal(const char* json, jsmntok_t* tok, const char* s);

static bool json_token_str_equal(const char* json, jsmntok_t* tok, const char* s)
{
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start
        && strncmp(json + tok->start, s, (size_t)(tok->end - tok->start)) == 0) {
        return true;
    }

    return false;
}

bool json_deserialize_time(char* buffer, size_t buffer_size, utc_command_t* utc_command)
{
    if (strlen(buffer) == 0)
        return false;

    char* string_part;
    uint64_t conversion_utc = strtoul(buffer, &string_part, 10);
    if (conversion_utc == '\0') {
        return false;
    }
    utc_command->utc = conversion_utc;

    return true;
}

bool json_create_topic(const char direction[TOPIC_DIRECTION_SIZE], const char device_key[DEVICE_KEY_SIZE],
                       const char message_type[TOPIC_MESSAGE_TYPE_SIZE], char topic[TOPIC_SIZE])
{
    topic[0] = '\0';
    strcat(topic, direction);
    strcat(topic, "/");
    strcat(topic, device_key);
    strcat(topic, "/");
    strcat(topic, message_type);

    return true;
}

size_t json_deserialize_feeds_value_message(char* buffer, size_t buffer_size, feed_t* feeds_received)
{
    uint8_t number_of_deserialized_feeds = 0;
    jsmn_parser parser;
    jsmntok_t tokens[JSON_TOKEN_SIZE];

    jsmn_init(&parser);
    int parser_result = jsmn_parse(&parser, buffer, buffer_size, tokens, WOLK_ARRAY_LENGTH(tokens));

    /* Received JSON must be valid, and top level element must be array */
    if (parser_result < 1 || tokens[0].type != JSMN_ARRAY || parser_result >= (int)WOLK_ARRAY_LENGTH(tokens)) {
        return false;
    }

    for (int i = 1; i < parser_result; ++i) { // at 1st position expects json object; 0 position is json array
        if (tokens[i].type == JSMN_OBJECT) {
            // object equals feed
            for (int j = i + 1; tokens[j].type != JSMN_OBJECT; ++j) { // at 2nd position expects string
                if (tokens[j].type == JSMN_STRING) {
                    // get timestamp
                    if (json_token_str_equal(buffer, &tokens[j], "timestamp")) {
                        char timestamp[UTC_MILLISECONDS_LENGTH] = "";

                        if (snprintf(timestamp, WOLK_ARRAY_LENGTH(timestamp), "%.*s",
                                     tokens[j + 1].end - tokens[j + 1].start, buffer + tokens[j + 1].start)
                            >= (int)WOLK_ARRAY_LENGTH(timestamp)) {
                            return false;
                        }
                        char* string_part;
                        feed_set_utc(feeds_received, strtoul(timestamp, &string_part, 10));
                    } else {
                        // get reference
                        if (snprintf(feeds_received->reference, WOLK_ARRAY_LENGTH(feeds_received->reference), "%.*s",
                                     tokens[j].end - tokens[j].start, buffer + tokens[j].start)
                            >= (int)WOLK_ARRAY_LENGTH(feeds_received->reference)) {
                            return false;
                        }
                        // get value
                        j++; // move one position to select value, regardless it's json type: PRIMITIVE or STRING
                        //TODO
                        // if (snprintf(feeds_received->data, WOLK_ARRAY_LENGTH(feeds_received->data[0]), "%.*s",
                        //              tokens[j].end - tokens[j].start, buffer + tokens[j].start)
                        //     >= (int)WOLK_ARRAY_LENGTH(feeds_received->data[0])) {
                        //     return false;
                        // }
                    }
                }
            }
            // move to next feed
            feeds_received++;
            number_of_deserialized_feeds++;
        }
    }
    return number_of_deserialized_feeds;
}

static bool serialize_feed(feed_t* feed, data_type_t type, size_t feed_element_size, char* buffer, size_t buffer_size)
{
    char data_buffer[PAYLOAD_SIZE] = "";

    for (size_t i = 0; i < feed_element_size; ++i) {
        strcat(data_buffer, feed->data[i]);

        if (i < feed_element_size - 1)
            strcat(data_buffer, ",");
    }
    
    bool value = snprintf(buffer, buffer_size, "[{\"%s\":%s%s%s,\"timestamp\":%ld}]", feed->reference,
                        type == NUMERIC ? "" : "\"", data_buffer, type == NUMERIC ? "" : "\"", feed_get_utc(feed))
                >= (int)buffer_size;
    if (feed_get_utc(feed) > 0 && value) {
        return false;
    }
    else if (feed_get_utc(feed) == 0
               && snprintf(buffer, buffer_size, "[{\"%s\":%s%s%s}]", feed->reference, type == NUMERIC ? "" : "\"",
                           data_buffer, type == NUMERIC ? "" : "\"")
                      >= (int)buffer_size) {
        return false;
    }

    return true;
}

static size_t serialize_feeds(feed_t* feeds, data_type_t type, size_t number_of_feeds, char* buffer, size_t buffer_size)
{
    WOLK_UNUSED(number_of_feeds);

    char data_buffer[PAYLOAD_SIZE] = "";
    strcat(buffer, JSON_ARRAY_LEFT_BRACKET);

    for (size_t i = 0; i < number_of_feeds; ++i) {
        // when it consists of more feeds with the same reference it has to have utc
        if (feed_get_utc(feeds) == 0)
            return false;

        if (snprintf(data_buffer, buffer_size, "{\"%s\":%s%s%s,\"timestamp\":%ld}%s", feeds->reference,
                     type == NUMERIC ? "" : "\"", feeds->data[0], type == NUMERIC ? "" : "\"", feed_get_utc(feeds),
                     (number_of_feeds > 1 && i < (number_of_feeds - 1)) ? "," : "")
            >= (int)buffer_size)
            return false;

        strcat(buffer, data_buffer);
        feeds++;
    }

    strcat(buffer, JSON_ARRAY_RIGHT_BRACKET);
    return true;
}

size_t json_serialize_feeds(feed_t* feeds, data_type_t type, size_t number_of_feeds, size_t feed_element_size,
                            char* buffer, size_t buffer_size)
{
    /* Sanity check */
    WOLK_ASSERT(number_of_feeds > 0);

    if (number_of_feeds > 1) {
        WOLK_UNUSED(feed_element_size);
        return serialize_feeds(feeds, type, number_of_feeds, buffer, buffer_size);
    } else {
        return serialize_feed(feeds, type, feed_element_size, buffer, buffer_size) ? 1 : 0;
    }
    return 1;
}

bool json_serialize_sync_time(const char* device_key, outbound_message_t* outbound_message)
{
    json_create_topic(JSON_D2P_TOPIC, device_key, JSON_SYNC_TIME_TOPIC, outbound_message->topic);
    strncpy(outbound_message->payload, "", PAYLOAD_SIZE);

    return true;
}

bool json_serialize_keep_alive_message(const char* device_key, outbound_message_t* outbound_message)
{
    outbound_message_init(outbound_message, "", "");

    /* Serialize topic */
    if (snprintf(outbound_message->topic, WOLK_ARRAY_LENGTH(outbound_message->topic), "ping/%s", device_key)
        >= (int)WOLK_ARRAY_LENGTH(outbound_message->topic)) {
        return false;
    }

    return true;
}
