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

#include "model/outbound_message.h"
#include "utility/wolk_utils.h"

void outbound_message_init(outbound_message_t* outbound_message, const char* topic, const char* payload)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(topic) <= TOPIC_SIZE);
    WOLK_ASSERT(strlen(payload) <= PAYLOAD_SIZE);

    strcpy(outbound_message->topic, topic);
    strcpy(outbound_message->payload, payload);
}

char* outbound_message_get_topic(outbound_message_t* outbound_message)
{
    return outbound_message->topic;
}

char* outbound_message_get_payload(outbound_message_t* outbound_message)
{
    return outbound_message->payload;
}
