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

#ifndef WOLK_UTILS_H
#define WOLK_UTILS_H

#include <assert.h>

#define WOLK_UNUSED(x) ((void)x)
#define WOLK_ASSERT(__e) ((__e) ? (void)0 : __assert_func (__FILE__, __LINE__, \
       __ASSERT_FUNC, #__e))

#define WOLK_ARRAY_LENGTH(x) (sizeof(x) / sizeof((x)[0]))

#ifndef NULL
#  define NULL 0
#endif

#define BOOL_TO_STR(b) ((b) ? "true" : "false")

#endif
