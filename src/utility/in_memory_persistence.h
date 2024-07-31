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

#ifndef IN_MEMORY_PERSISTENCE_H
#define IN_MEMORY_PERSISTENCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "model/outbound_message.h"
#include "model/outbound_message.h"
#include "utility/circular_buffer.h"
#include "utility/wolk_utils.h"


void in_memory_persistence_init(void* storage, uint32_t num_elements, bool wrap);

bool in_memory_persistence_push(outbound_message_t* outbound_message);

bool in_memory_persistence_peek(outbound_message_t* outbound_message);

bool in_memory_persistence_pop(outbound_message_t* outbound_message);

bool in_memory_persistence_is_empty(void);

#ifdef __cplusplus
}
#endif

#endif
