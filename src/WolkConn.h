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
/**
 *@file
 *@brief Header file with library function descriptions 
 */

#ifndef WOLK_H
#define WOLK_H
/** @cond */
#include "MQTTClient.h"
#include "utility/WolkQueue.h"
#include "protocol/parser.h"
#include "utility/size_definitions.h"
#include "model/outbound_message.h"
#include "model/outbound_message_factory.h"
#include "utility/wolk_utils.h"
#include "model/parameter.h"

#include "utility/in_memory_persistence.h"
#include "utility/persistence.h"
#include "model/feed.h"

#include "Arduino.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
/** @endcond */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WOLK_ERR_T Boolean used for error handling in Wolk connection module
 */
typedef unsigned char WOLK_ERR_T;
/**
 * @brief WOLK_ERR_T Boolean used in Wolk connection module
 */
typedef unsigned char WOLK_BOOL_T;
enum WOLK_BOOL_T_values { W_FALSE = 0, W_TRUE = 1 };

/**
 * @brief  WolkAbout IoT Platform numeric feed type.
 */
typedef struct wolk_numeric_feeds_t {
    double value;
    uint64_t utc_time;
} wolk_numeric_feeds_t;

/**
 * @brief  WolkAbout IoT Platform string feed type.
 */
typedef struct wolk_string_feeds_t {
    char* value;
    uint64_t utc_time;
} wolk_string_feeds_t;

/**
 * @brief  WolkAbout IoT Platform boolean feed type.
 */
typedef struct wolk_boolean_feeds_t {
    bool value;
    uint64_t utc_time;
} wolk_boolean_feeds_t;

typedef feed_t wolk_feed_t;
typedef feed_registration_t wolk_feed_registration_t;
typedef parameter_t wolk_parameter_t;
typedef attribute_t wolk_attribute_t;

/**
 * @brief Declaration of feed value handler.
 *
 * @param feeds feeds received as name:value pairs from WolkAbout IoT Platform.
 * @param number_of_feeds number fo received feeds.
 */
typedef void (*feed_handler_t)(wolk_feed_t* feeds, size_t number_of_feeds);

/**
 * @brief Declaration of parameter handler.
 *
 * @param parameter_message Parameters received as name:value pairs from WolkAbout IoT Platform.
 * @param number_of_parameters number of received parameters
 */
typedef void (*parameter_handler_t)(wolk_parameter_t* parameter_message, size_t number_of_parameters);

/**
 * @brief Declaration of details synchronization handler. It will be called as a response on the
 * wolk_details_synchronization() call. It will give list of the all feeds and attributes from the platform.
 *
 * @param parameter_message Parameters received as name:value pairs from WolkAbout IoT Platform.
 * @param number_of_parameters number of received parameters
 */
typedef void (*details_synchronization_handler_t)(wolk_feed_registration_t* feeds, size_t number_of_received_feeds,
                                                  wolk_attribute_t* attributes, size_t number_of_received_attributes);


typedef struct _wolk_ctx_t wolk_ctx_t;

struct _wolk_ctx_t {
    int sock;
    PubSubClient *mqtt_client;

    outbound_mode_t outbound_mode;

    feed_handler_t feed_handler; /**< Callback for handling incoming feeds from WolkAbout IoT Platform.
                                              @see feed_handler_t*/

    parameter_handler_t parameter_handler; /**< Callback for handling received configuration from WolkAbout IoT
                                                      Platform. @see parameter_handler_t*/

    details_synchronization_handler_t details_synchronization_handler; /**< Callback for handling received details
configuration from WolkAbout IoT Platform. @see attribute_handler_t*/

    char device_key[DEVICE_KEY_SIZE];                       /**<  Authentication parameters for WolkAbout IoT Platform. Obtained as a result of device creation on the platform.*/
    char device_password[DEVICE_PASSWORD_SIZE];             /**<  Authentication parameters for WolkAbout IoT Platform. Obtained as a result of device creation on the platform.*/

    parser_t parser;

    persistence_t persistence;

    bool is_keep_alive_enabled;
    unsigned long millis_last_ping;

    bool is_initialized;
    bool is_connected;
    bool pong_received;

    unsigned long epoch_time;                                    /**< Epoch time in milliseconds, is updated on every ping but can be updated manually by calling wolk_update_epoch()
                                                            Since this is updated by receiving the value from the platform, wolk_process must also be called*/
};

//TODO: continue
/**
 * @brief Initializes WolkAbout IoT Platform connector context
 * @param ctx Context
 *
 * @param device_key Device key provided by WolkAbout IoT Platform upon device
 * creation
 * @param password Device password provided by WolkAbout IoT Platform device
 * upon device creation
 * @param client MQQT Client
 * @param server MQQT Server
 * @param port Port to connect to
 * 
 * @return Error code
 */
WOLK_ERR_T wolk_init(wolk_ctx_t* ctx,
                    const char* device_key, const char* device_password, PubSubClient *client, const char *server, int port,
                    outbound_mode_t outbound_mode, feed_handler_t feed_handler,
                    parameter_handler_t parameter_handler,
                    details_synchronization_handler_t details_synchronization_handler);
/**
 * @brief Initializes persistence mechanism with in-memory implementation
 *
 * @param ctx Context
 * @param storage Address to start of the memory which will be used by
 * persistence mechanism
 * @param size Size of memory in bytes
 * @param wrap If storage is full overwrite oldest item when pushing new item
 *
 * @return Error code
 */
WOLK_ERR_T wolk_init_in_memory_persistence(wolk_ctx_t* ctx, void* storage, uint32_t size, bool wrap);
/**
 * @brief Initializes persistence mechanism with custom implementation
 *
 * @param ctx Context
 * @param push Function pointer to 'push' implemenation
 * @param peek Function pointer to 'peek' implementation
 * @param pop Function pointer to 'pop' implementation
 * @param is_empty Function pointer to 'is empty' implementation
 *
 * @return Error code
 *
 * @see persistence.h for signatures of methods to be implemented, and
 * implementation contract
 */
WOLK_ERR_T wolk_init_custom_persistence(wolk_ctx_t* ctx, persistence_push_t push, persistence_peek_t peek,
                                        persistence_pop_t pop, persistence_is_empty_t is_empty);


/**
 * @brief Connect to WolkAbout IoT Platform
 *
 *  1. Context must be initialized via wolk_init()
 *  2. Persistence must be initialized using
 *      wolk_initialize_in_memory_persistence(wolk_ctx_t *ctx, void* storage,
 * uint16_t num_elements, bool wrap) or
 *      wolk_initialize_custom_persistence(wolk_ctx_t *ctx,
 *                                         persistence_push_t push,
 * persistence_pop_t pop, persistence_is_empty_t is_empty, persistence_size_t
 * size)
 * @param ctx Context
 *
 * @return Error code
 */
WOLK_ERR_T wolk_connect (wolk_ctx_t *ctx);

/** @brief Disconnect from WolkAbout IoT Platform
 *
 *  @param ctx library context
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_disconnect (wolk_ctx_t *ctx);

/**
 * @brief Must be called periodically to keep the connection to the WolkAbout IoT
 * platform alive, obtain and perform feeds. It also serves as a diagnostic tool
 * for the MQTT client, printing it's state when an error occurs.
 * Client state will be -3 if the connection is lost and -4 if the broker can't be reached. 
 *
 * @param ctx Context
 *
 * @return Error code
 */
WOLK_ERR_T wolk_process (wolk_ctx_t *ctx);

/** @brief Add string feed
 *
 * @param ctx Context
 * @param reference Feed reference
 * @param feeds Feed values, one or more values organized as value:utc pairs. Value is char pointer. Utc time has to
 * be in milliseconds.
 * @param number_of_feeds Number of feeds that is captured
 *
 *  @return Error code
 */
WOLK_ERR_T wolk_add_string_feed(wolk_ctx_t* ctx, const char* reference, wolk_string_feeds_t* feeds,
                                size_t number_of_feeds);

/**
 * @brief Add numeric feeds
 *
 * @param ctx Context
 * @param reference Feed reference
 * @param feeds Feed values, one or more values organized as value:utc pairs. Value is double. Utc time has to be in
 * milliseconds.
 * @param number_of_feeds Number of feeds that is captured
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_numeric_feed(wolk_ctx_t* ctx, const char* reference, wolk_numeric_feeds_t* feeds,
                                 size_t number_of_feeds);

/**
 * @brief Add multi-value numeric feed. For feeds that has more than one numeric number associated as value, like
 * location is. Max number of numeric values is define with FEEDS_MAX_NUMBER from size_definition
 *
 * @param ctx Context
 * @param reference Feed reference
 * @param values Feed values
 * @param value_size Number of numeric values limited by FEEDS_MAX_NUMBER
 * @param utc_time UTC time of feed value acquisition [miliseconds]
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_multi_value_numeric_feed(wolk_ctx_t* ctx, const char* reference, double* values,
                                             uint16_t value_size, uint64_t utc_time);

/**
 * @brief Add bool feed
 *
 * @param ctx Context
 * @param reference Feed reference
 * @param feeds Feed values, one or more values organized as value:utc pairs. Value is boolean. Utc time has to be in
 * milliseconds.
 * @param number_of_feeds Number of feeds that is captured
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_bool_feeds(wolk_ctx_t* ctx, const char* reference, wolk_boolean_feeds_t* feeds,
                               size_t number_of_feeds);

/**
 * @brief Publish accumulated sensor readings, and alarms
 *
 * @param ctx Context
 *
 * @return Error code
 */
WOLK_ERR_T wolk_publish(wolk_ctx_t* ctx);

/**
 * @brief Disables internal keep alive mechanism
 *
 * @param ctx Context
 *
 * @return Error code
 */
WOLK_ERR_T wolk_disable_keep_alive(wolk_ctx_t* ctx);

/**
 * @brief Requests the epoch time from the platform and awaits reply.
 *  If the reply did not arrive within 60 seconds it returns W_TRUE.
 * 
 * @param ctx Context
 *
 * @return Error code
 */
WOLK_ERR_T wolk_update_epoch(wolk_ctx_t* ctx);

/**
 * @brief Get last received UTC from platform
 *
 * @param ctx Context
 *
 * @return UTC in miliseconds
 */
uint64_t wolk_request_timestamp(wolk_ctx_t* ctx);

#ifdef __cplusplus
}
#endif

#endif

