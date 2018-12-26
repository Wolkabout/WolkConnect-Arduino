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

#include "in_memory_persistence.h"
#include "circular_buffer.h"
#include "outbound_message.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stdint.h>

static circular_buffer_t buffer;

void in_memory_persistence_init(void* storage, uint32_t size, bool wrap)
{
    uint32_t num_elements = size / sizeof(outbound_message_t);
    WOLK_ASSERT(num_elements > 0);

    circular_buffer_init(&buffer, storage, num_elements, sizeof(outbound_message_t), wrap, true);
}

bool in_memory_persistence_push(outbound_message_t* outbound_message)
{
    return circular_buffer_add(&buffer, outbound_message);
}

bool in_memory_persistence_peek(outbound_message_t* outbound_message)
{
    return circular_buffer_peek(&buffer, 0, outbound_message);
}

bool in_memory_persistence_pop(outbound_message_t* outbound_message)
{
    return circular_buffer_pop(&buffer, outbound_message);
}

bool in_memory_persistence_is_empty()
{
    return circular_buffer_empty(&buffer);
}
