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

#ifndef OUTBOUND_MESSAGE_H
#define OUTBOUND_MESSAGE_H

#include "size_definitions.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char topic[TOPIC_SIZE];
    char payload[PAYLOAD_SIZE];
} outbound_message_t;

void outbound_message_init(outbound_message_t* outbound_message, const char* topic, const char* payload);

char* outbound_message_get_topic(outbound_message_t* outbound_message);

char* outbound_message_get_payload(outbound_message_t* outbound_message);

#ifdef __cplusplus
}
#endif

#endif
