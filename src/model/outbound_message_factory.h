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

#ifndef OUTBOUND_MESSAGE_FACTORY_H
#define OUTBOUND_MESSAGE_FACTORY_H

#include <stdbool.h>

#include "model/feed.h"
#include "model/outbound_message.h"
#include "protocol/parser.h"
#include "utility/size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

size_t outbound_message_make_from_feeds(parser_t* parser, const char* device_key, feed_t* feeds, data_type_t type,
                                        size_t feeds_number, size_t feeds_element_size,
                                        outbound_message_t* outbound_message);
bool outbound_message_synchronize_time(parser_t* parser, const char* device_key, outbound_message_t* outbound_message);
bool outbound_message_make_from_keep_alive_message(parser_t* parser, const char* device_key,
                                                   outbound_message_t* outbound_message);

#ifdef __cplusplus
}
#endif

#endif
