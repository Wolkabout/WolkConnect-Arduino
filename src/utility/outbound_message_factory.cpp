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

#include "outbound_message_factory.h"
#include "outbound_message.h"
#include "parser.h"
#include "reading.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

size_t outbound_message_make_from_readings(parser_t* parser, const char* device_key, reading_t* first_reading,
                                           size_t num_readings, outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    WOLK_ASSERT(first_reading);
    WOLK_ASSERT(outbound_message);

    char topic[TOPIC_SIZE];
    char payload[PAYLOAD_SIZE];
    size_t num_serialized = 0;

    memset(topic, '\0', sizeof(topic));
    memset(payload, '\0', sizeof(payload));

    if (!parser_serialize_readings_topic(parser, device_key, first_reading, num_readings, topic, sizeof(topic))) {
        return num_serialized;
    }

    num_serialized = parser_serialize_readings(parser, first_reading, num_readings, payload, sizeof(payload));
    outbound_message_init(outbound_message, topic, payload);
    return num_serialized;
}

bool outbound_message_make_from_actuator_status(parser_t* parser, const char* device_key,
                                                actuator_status_t* actuator_status, const char* reference,
                                                outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    WOLK_ASSERT(actuator_status);
    WOLK_ASSERT(reference);
    WOLK_ASSERT(outbound_message);

    manifest_item_t manifest_item;
    manifest_item_init(&manifest_item, reference, READING_TYPE_ACTUATOR, DATA_TYPE_STRING);

    reading_t reading;
    reading_init(&reading, &manifest_item);
    reading_set_data(&reading, actuator_status_get_value(actuator_status));
    reading_set_actuator_state(&reading, actuator_status_get_state(actuator_status));


    char topic[TOPIC_SIZE];
    memset(topic, '\0', sizeof(topic));
    if (!parser_serialize_readings_topic(parser, device_key, &reading, 1, topic, sizeof(topic))) {
        return false;
    }

    char payload[PAYLOAD_SIZE];
    memset(payload, '\0', sizeof(payload));
    if (parser_serialize_readings(parser, &reading, 1, payload, sizeof(payload)) == 0) {
        return false;
    }

    outbound_message_init(outbound_message, topic, payload);
    return true;
}

bool outbound_message_make_from_configuration(parser_t* parser, const char* device_key,
                                              char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                              char (*value)[CONFIGURATION_VALUE_SIZE], size_t num_configuration_items,
                                              outbound_message_t* outbound_message)
{
    /* Sanity check */
    WOLK_ASSERT(parser);
    WOLK_ASSERT(device_key);
    WOLK_ASSERT(reference);
    WOLK_ASSERT(value);
    WOLK_ASSERT(outbound_message);

    return parser_serialize_configuration(parser, device_key, reference, value, num_configuration_items,
                                          outbound_message);
}

//bool outbound_message_make_from_firmware_update_status(parser_t* parser, const char* device_key,
//                                                       firmware_update_status_t* firmware_update_status,
//                                                       outbound_message_t* outbound_message)
//{
//    /* Sanity check */
//    WOLK_ASSERT(parser);
//    WOLK_ASSERT(device_key);
//    WOLK_ASSERT(firmware_update_status);
//    WOLK_ASSERT(outbound_message);
//
//    return parser_serialize_firmware_update_status(parser, device_key, firmware_update_status, outbound_message);
//}
//
//bool outbound_message_make_from_firmware_update_packet_request(
//    parser_t* parser, const char* device_key, firmware_update_packet_request_t* firmware_update_packet_request,
//    outbound_message_t* outbound_message)
//{
//    /* Sanity check */
//    WOLK_ASSERT(parser);
//    WOLK_ASSERT(firmware_update_packet_request);
//    WOLK_ASSERT(outbound_message);
//
//    return parser_serialize_firmware_update_packet_request(parser, device_key, firmware_update_packet_request,
//                                                           outbound_message);
//}
//
//bool outbound_message_make_from_firmware_version(parser_t* parser, const char* device_key, const char* version,
//                                                 outbound_message_t* outbound_message)
//{
//    /* Sanity check */
//    WOLK_ASSERT(parser);
//    WOLK_ASSERT(device_key);
//    WOLK_ASSERT(version);
//
//    return parser_serialize_firmware_update_version(parser, device_key, version, outbound_message);
//}
//
//bool outbound_message_make_from_keep_alive_message(parser_t* parser, const char* device_key,
//                                                   outbound_message_t* outbound_message)
//{
//    /* Sanity check */
//    WOLK_ASSERT(parser);
//    WOLK_ASSERT(device_key);
//    WOLK_ASSERT(outbound_message);
//
//    return parser_serialize_keep_alive_message(parser, device_key, outbound_message);
//}
