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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* Maximum number of characters in device key string */
    DEVICE_KEY_SIZE = 32,

    /* Maximum number of characters in device password string */
    DEVICE_PASSWORD_SIZE = 40,

    /* Maximum number of characters in topic string */
    TOPIC_SIZE = 64,
    /* Maximum number of bytes in payload string */
    PAYLOAD_SIZE = 256,

    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_REFERENCE_SIZE = 64,

    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_DATA_DELIMITER_SIZE= 3,

    /* Maximum number of characters in reading value string */
    READING_SIZE = 96,
    /* Maximum number of reading dimensions (Data size on DV-Tool) */
    READING_DIMENSIONS = 3,

    /* Maximum number of characters in command name */
    COMMAND_MAX_SIZE = 15,
    /* Maximum number of characters in actuation value string */
    COMMAND_ARGUMENT_SIZE = READING_SIZE,

    /* Maximum number of characters in configuration item name string */
    CONFIGURATION_REFERENCE_SIZE = MANIFEST_ITEM_REFERENCE_SIZE,
    /* Maximum number of characters in configuration item value string */
    CONFIGURATION_VALUE_SIZE = 128,
    /* Maximum number of configuration items for device */
    CONFIGURATION_ITEMS_SIZE = 3,

    /* Parser internal buffer size, should be at least READING_SIZE  big */
    PARSER_INTERNAL_BUFFER_SIZE = READING_SIZE,

    /*used in publish actuator status*/
    READINGS_BUFFER_SIZE = 192,

    STORE_SIZE = 16

};

#ifdef __cplusplus
}
#endif

#endif
