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

#ifndef PARAMETER_H
#define PARAMETER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "utility/size_definitions.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char name[PARAMETER_TYPE_SIZE];
    char value[PARAMETER_VALUE_SIZE];
} parameter_t;

void parameter_init(parameter_t* parameter_message, char* name, char* value);

void parameter_set_value(parameter_t* parameter_message, char* buffer);

#ifdef __cplusplus
}
#endif

#endif
