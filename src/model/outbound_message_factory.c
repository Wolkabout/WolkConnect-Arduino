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

#include "model/outbound_message_factory.h"


size_t outbound_message_make_from_feeds(parser_t* parser, const char* device_key, feed_t* feeds, data_type_t type,
                                        size_t feeds_number, size_t feeds_element_size,
                                        outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    WOLK_ASSERT(feeds);
    WOLK_ASSERT(feeds_number);
    WOLK_ASSERT(feeds_element_size);
    WOLK_ASSERT(outbound_message);

    char topic[TOPIC_SIZE] = "";
    char payload[PAYLOAD_SIZE] = "";
    size_t num_serialized = 0;

    parser->create_topic(parser->D2P_TOPIC, device_key, parser->FEED_VALUES_MESSAGE_TOPIC, topic);

    num_serialized =
        parser_serialize_feeds(parser, feeds, type, feeds_number, feeds_element_size, payload, sizeof(payload));
    if (num_serialized != 0)
        outbound_message_init(outbound_message, topic, payload);

    return num_serialized;
}

bool outbound_message_synchronize_time(parser_t* parser, const char* device_key, outbound_message_t* outbound_message)
{
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    return parser_serialize_sync_time(parser, device_key, outbound_message);
}

bool outbound_message_make_from_keep_alive_message(parser_t* parser, const char* device_key,
                                                   outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    WOLK_ASSERT(outbound_message);

    // return parser_serialize_keep_alive_message(parser, device_key, outbound_message);
    return true;
}
