/*
 * Copyright 2022 WolkAbout Technology s.r.o.
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

#include "model/feed.h"


void feed_initialize(feed_t* feed, uint16_t feed_size, const char* reference)
{
    feed->size = feed_size > FEEDS_MAX_NUMBER ? FEEDS_MAX_NUMBER : feed_size;

    feed->utc = 0;

    feed_clear(feed);
    strcpy(feed->reference, reference);
}

void feed_clear(feed_t* feed)
{
    for (size_t i = 0; i < feed->size; ++i) {
        feed_set_data_at(feed, "", i);
    }
}

void feed_set_data(feed_t* feed, const char** data)
{
    for (size_t i = 0; i < feed->size; ++i) {
        WOLK_ASSERT(strlen(data[i]) < FEED_ELEMENT_SIZE);

        strcpy(feed->data[i], data[i]);
    }
}

void feed_set_data_at(feed_t* feed, const char* data, size_t data_position)
{
    /* Sanity check */
    WOLK_ASSERT(strlen(data) < FEED_ELEMENT_SIZE);

    strcpy(feed->data[data_position], data);
}

char** feed_get_data(feed_t* feed)
{
    // return feed->data;
}

char* feed_get_data_at(feed_t* feed, size_t data_position)
{
    /* Sanity check */
    WOLK_ASSERT(data_position < feed->manifest_item.data_dimensions);

    return feed->data[data_position];
}

void feed_set_utc(feed_t* feed, uint64_t utc)
{
    feed->utc = utc;
}

uint64_t feed_get_utc(feed_t* feed)
{
    return feed->utc;
}

void feed_initialize_registration(feed_registration_t* feed, char* name, const char* reference, char* unit,
                                  const feed_type_t feedType)
{
    strncpy(feed->name, name, ITEM_NAME_SIZE);
    strncpy(feed->reference, reference, REFERENCE_SIZE);
    strncpy(feed->unit, unit, ITEM_UNIT_SIZE);
    feed->feedType = feedType;
}
