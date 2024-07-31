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

#ifndef FEED_H
#define FEED_H

#include "utility/size_definitions.h"
#include "utility/wolk_utils.h"
#include "utility/wolk_types.h"

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char name[ITEM_NAME_SIZE];
    char reference[REFERENCE_SIZE];
    char unit[ITEM_UNIT_SIZE];

    feed_type_t feedType;
} feed_registration_t;

typedef struct {
    char data[FEEDS_MAX_NUMBER][FEED_ELEMENT_SIZE];
    uint16_t size;

    char reference[REFERENCE_SIZE];

    uint64_t utc;
} feed_t;

void feed_initialize(feed_t* feed, uint16_t feed_size, const char* reference);

void feed_clear(feed_t* feed);

void feed_set_data(feed_t* feed, const char** data);

void feed_set_data_at(feed_t* feed, const char* data, size_t data_position);
char* feed_get_data_at(feed_t* feed, size_t data_position);

void feed_set_utc(feed_t* feed, uint64_t utc);
uint64_t feed_get_utc(feed_t* feed);

void feed_initialize_registration(feed_registration_t* feed, char* name, const char* reference, char* unit,
                                  feed_type_t feedType);

#ifdef __cplusplus
}
#endif

#endif // FEED_H
