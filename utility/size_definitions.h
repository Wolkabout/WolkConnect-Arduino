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

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_REFERENCE_SIZE = 64,
    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_DATA_DELIMITER_SIZE= 5,

    /* Maximum number of characters in reading value string */
    READING_SIZE = 96,
    /* Maximum number of reading dimensions (Data size on DV-Tool) */
    READING_DIMENSIONS = 3,

    /* Maximum number of characters in actuation value string */
    COMMAND_ARGUMENT_SIZE = READING_SIZE,

    /* Maximum number of characters in configuration item name string */
    CONFIGURATION_ITEM_NAME_SIZE = MANIFEST_ITEM_REFERENCE_SIZE,
    /* Maximum number of characters in configuration item value string */
    CONFIGURATION_ITEM_VALUE_SIZE = READING_SIZE,

    /* Parser internal buffer size, should be at least READING_SIZE  big */
    PARSER_INTERNAL_BUFFER_SIZE = READING_SIZE
};

#ifdef __cplusplus
}
#endif

#endif
