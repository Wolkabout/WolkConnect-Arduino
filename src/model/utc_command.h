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

#ifndef UTC_COMMAND_H
#define UTC_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    unsigned long utc;
} utc_command_t;

unsigned long utc_command_get(utc_command_t* utc_command);

#ifdef __cplusplus
}
#endif

#endif // UTC_COMMAND_H
