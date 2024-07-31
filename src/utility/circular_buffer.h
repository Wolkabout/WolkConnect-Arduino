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

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t head;
    uint32_t tail;

    void* storage;

    uint32_t storage_size;
    uint32_t element_size;

    bool empty;
    bool full;
    bool wrap;
} circular_buffer_t;

void circular_buffer_init(circular_buffer_t* circular_buffer, void* storage, uint32_t storage_size,
                          uint32_t element_size, bool wrap, bool clear);

bool circular_buffer_add(circular_buffer_t* buffer, const void* element);

bool circular_buffer_add_array(circular_buffer_t* buffer, const void* elements_array, uint32_t length);

uint32_t circular_buffer_add_as_many_as_possible(circular_buffer_t* buffer, const void* elements_array,
                                                 uint32_t length);

bool circular_buffer_pop(circular_buffer_t* buffer, void* element);

uint32_t circular_buffer_pop_array(circular_buffer_t* buffer, uint32_t length, void* elements_array);

uint32_t circular_buffer_drop_from_beggining(circular_buffer_t* buffer, uint32_t number_of_elements);

uint32_t circular_buffer_drop_from_end(circular_buffer_t* buffer, uint32_t number_of_elements);

bool circular_buffer_peek(circular_buffer_t* buffer, uint32_t element_position, void* element);

uint32_t circular_buffer_peek_array(circular_buffer_t* buffer, uint32_t element_position, uint32_t length,
                                    void* elements_array);

bool circular_buffer_empty(circular_buffer_t* buffer);

bool circular_buffer_full(circular_buffer_t* buffer);

uint32_t circular_buffer_size(circular_buffer_t* buffer);

uint32_t circular_buffer_free_space(circular_buffer_t* buffer);

void circular_buffer_clear(circular_buffer_t* buffer);

#ifdef __cplusplus
}
#endif
#endif