/*
 * Copyright 2017 WolkAbout Technology s.r.o.
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

#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "actuator_command.h"
#include "configuration_item.h"
#include "configuration_item_command.h"
#include "outbound_message.h"
#include "reading.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t json_serialize_readings(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);

size_t json_deserialize_commands(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);

size_t json_serialize_configuration_items(configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size);

size_t json_deserialize_configuration_items(char* buffer, size_t buffer_size, configuration_item_command_t* commands_buffer, size_t commands_buffer_size);

bool json_serialize_readings_topic(reading_t* first_Reading, size_t num_readings, const char* device_key, char* buffer,
                                   size_t buffer_size);
#ifdef __cplusplus
}
#endif

#endif
