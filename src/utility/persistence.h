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

#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "model/outbound_message.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief persistence_push signature.
 * Pushes item to persistence.
 *
 * @return true if item was successfully pushed to persistence, false otherwise
 */
typedef bool (*persistence_push_t)(outbound_message_t*);

/**
 * @brief persistence_peek signature.
 * Peeks item from persistence
 *
 * @return true if item was successfully peeked from persistence, false
 * otherwise
 */
typedef bool (*persistence_peek_t)(outbound_message_t*);

/**
 * @brief persistence_pop signature.
 * Pops item from persistence
 *
 * @return true if item was successfully popped from persistence, false
 * otherwise
 */
typedef bool (*persistence_pop_t)(outbound_message_t*);

/**
 * @brief persistence_is_empty signature.
 * Checks whether persistence contains item(s)
 *
 * @return true if persistence contains item(s), false otherwise
 */
typedef bool (*persistence_is_empty_t)(void);

typedef struct {
    persistence_push_t push;
    persistence_peek_t peek;
    persistence_pop_t pop;
    persistence_is_empty_t is_empty;

    bool is_initialized;
} persistence_t;

void persistence_init(persistence_t* persistence, persistence_push_t push, persistence_peek_t peek,
                      persistence_pop_t pop, persistence_is_empty_t is_empty);

bool persistence_is_initialized(const persistence_t* persistence);

bool persistence_push(persistence_t* persistence, outbound_message_t* item);

bool persistence_peek(persistence_t* persistence, outbound_message_t* item);

bool persistence_pop(persistence_t* persistence, outbound_message_t* item);

bool persistence_is_empty(persistence_t* persistence);

#ifdef __cplusplus
}
#endif

#endif