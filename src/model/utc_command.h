/*
 * Copyright 2020 WolkAbout Technology s.r.o.
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

#ifndef WOLKCONNECTOR_C_UTC_COMMAND_H
#define WOLKCONNECTOR_C_UTC_COMMAND_H

#include <stdint.h>

typedef struct {
    uint64_t utc;
} utc_command_t;

uint64_t utc_command_get(utc_command_t* utc_command);

#endif // WOLKCONNECTOR_C_UTC_COMMAND_H
