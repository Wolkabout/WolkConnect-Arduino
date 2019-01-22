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

#include "circular_buffer.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static void increase_pointer(uint32_t* pointer, uint32_t storage_size)
{
    if ((*pointer) == (storage_size - 1)) {
        (*pointer) = 0;
    } else {
        (*pointer)++;
    }
}

static void decrease_pointer(uint32_t* pointer, uint32_t storage_size)
{
    if ((*pointer) == 0) {
        (*pointer) = storage_size - 1;
    } else {
        (*pointer)--;
    }
}

void circular_buffer_init(circular_buffer_t* circular_buffer, void* storage, uint32_t storage_size,
                          uint32_t element_size, bool wrap, bool clear)
{
    circular_buffer->storage = storage;
    circular_buffer->storage_size = storage_size;
    circular_buffer->element_size = element_size;
    circular_buffer->wrap = wrap;
    if (clear) {
        circular_buffer_clear(circular_buffer);
    }
}

static void copy_bytes(void* destination, uint32_t destination_offset, const void* source, uint32_t source_offset,
                       uint32_t data_length)
{
    unsigned char* destination_position = (unsigned char*)destination + destination_offset * data_length;
    const unsigned char* source_position = (const unsigned char*)source + source_offset * data_length;
    memcpy(destination_position, source_position, data_length);
}

bool circular_buffer_add(circular_buffer_t* buffer, const void* element)
{
    if (!buffer || !element) {
        return false;
    }

    if (buffer->full) {
        if (buffer->wrap) {
            /* if buffer is full the oldest element will be discarded, new one
             * will come to its place */
            increase_pointer(&buffer->head, buffer->storage_size);
        } else {
            return false;
        }
    }

    copy_bytes(buffer->storage, buffer->tail, element, 0, buffer->element_size);

    increase_pointer(&buffer->tail, buffer->storage_size);

    /* we added something so buffer is for sure not empty anymore */
    buffer->empty = false;
    /* but it can happen that it is full */
    buffer->full = (buffer->tail == buffer->head);

    return true;
}

/**
 * Adds element to buffer. If buffer is full it will overwrite the oldest
 * element.
 */
bool circular_buffer_add_array(circular_buffer_t* buffer, const void* elements_array, uint32_t length)
{
    uint32_t free_space;
    uint64_t i = 0;

    if (!buffer || !elements_array) {
        return false;
    }

    free_space = circular_buffer_free_space(buffer);
    if (!buffer->wrap && (length > free_space)) {
        return false;
    }

    for (i = 0; i < length; i++) {
        const unsigned char* source_position = (const unsigned char*)elements_array + i * buffer->element_size;
        circular_buffer_add(buffer, source_position);
    }

    return true;
}

uint32_t circular_buffer_add_as_many_as_possible(circular_buffer_t* buffer, const void* elements_array, uint32_t length)
{
    uint32_t to_add;
    uint64_t i;

    if (!buffer || !elements_array) {
        return 0;
    }

    to_add = circular_buffer_free_space(buffer);
    if (length < to_add) {
        to_add = length;
    }

    for (i = 0; i < to_add; i++) {
        const unsigned char* source_position = (const unsigned char*)elements_array + i * buffer->element_size;
        circular_buffer_add(buffer, source_position);
    }

    return to_add;
}

bool circular_buffer_pop(circular_buffer_t* buffer, void* element)
{
    if (!buffer || buffer->empty) {
        return false;
    }

    if (element) {
        copy_bytes(element, 0, buffer->storage, buffer->head, buffer->element_size);
    }

    increase_pointer(&buffer->head, buffer->storage_size);

    /* it is for sure not full any more since we removed something */
    buffer->full = false;
    /* might be empty */
    buffer->empty = (buffer->tail == buffer->head);

    return true;
}

/**
 * Drops first number_of_elements.
 */

uint32_t circular_buffer_pop_array(circular_buffer_t* buffer, uint32_t length, void* elements_array)
{
    uint32_t read;

    if (!buffer) {
        return 0;
    }

    read = 0;
    unsigned char* elements_array_position = elements_array ? (unsigned char*)elements_array : NULL;
    while ((read < length) && circular_buffer_pop(buffer, elements_array_position)) {
        read++;
        if (elements_array_position) {
            elements_array_position += buffer->element_size;
        }
    }

    return read;
}

uint32_t circular_buffer_drop_from_beggining(circular_buffer_t* buffer, uint32_t number_of_elements)
{
    return circular_buffer_pop_array(buffer, number_of_elements, NULL);
}

uint32_t circular_buffer_drop_from_end(circular_buffer_t* buffer, uint32_t number_of_elements)
{
    uint64_t i;

    if (!buffer) {
        return 0;
    }
    if(number_of_elements >= circular_buffer_size(buffer)) {
        return 0;
    }
    for (i = 0; i < number_of_elements; i++) {
        decrease_pointer(&buffer->tail, buffer->storage_size);
        buffer->full = false;
    }

    return number_of_elements;
}

bool circular_buffer_peek(circular_buffer_t* buffer, uint32_t element_position, void* element)
{
    if (!buffer || !element) {
        return false;
    }

    if (buffer->empty || (element_position >= circular_buffer_size(buffer))) {
        return false;
    }

    if (buffer->head + element_position < buffer->storage_size) {
        copy_bytes(element, 0, buffer->storage, buffer->head + element_position, buffer->element_size);
    } else {
        copy_bytes(element, 0, buffer->storage, buffer->head + element_position - buffer->storage_size,
                   buffer->element_size);
    }

    return true;
}

uint32_t circular_buffer_peek_array(circular_buffer_t* buffer, uint32_t element_position, uint32_t length,
                                    void* elements_array)
{
    uint32_t read;

    if (!buffer || !elements_array) {
        return false;
    }

    read = 0;
    unsigned char* elements_array_position = (unsigned char*)elements_array;
    while ((read < length)
           && circular_buffer_peek(buffer, element_position + read,
                                   elements_array_position + read * buffer->element_size)) {
        read++;
    }

    return read;
}

bool circular_buffer_empty(circular_buffer_t* buffer)
{
    if (!buffer) {
        return true;
    }

    return buffer->empty;
}

bool circular_buffer_full(circular_buffer_t* buffer)
{
    if (!buffer) {
        return false;
    }

    return buffer->full;
}

uint32_t circular_buffer_size(circular_buffer_t* buffer)
{
    if (!buffer || buffer->empty) {
        return 0;
    }

    if (buffer->full) {
        return buffer->storage_size;
    }

    if (buffer->tail > buffer->head) {
        return buffer->tail - buffer->head;
    }

    return buffer->storage_size - buffer->head + buffer->tail;
}

uint32_t circular_buffer_free_space(circular_buffer_t* buffer)
{
    if (!buffer) {
        return 0;
    }

    uint32_t size;

    size = circular_buffer_size(buffer);
    return buffer->storage_size - size;
}

void circular_buffer_clear(circular_buffer_t* buffer)
{
    if (buffer) {
        buffer->head = 0;
        buffer->tail = 0;
        buffer->empty = true;
        buffer->full = false;
        memset(buffer->storage, 0, buffer->storage_size * buffer->element_size);
    }
}