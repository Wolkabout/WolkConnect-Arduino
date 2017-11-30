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

#ifndef WOLK_H
#define WOLK_H

#include "MQTTClient.h"
#include "utility/WolkQueue.h"
#include "utility/parser.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WOLK_ERR_T Boolean used for error handling in Wolk conenction module
 */
typedef unsigned char WOLK_ERR_T;
/**
 * @brief WOLK_ERR_T Boolean used in Wolk connection module
 */
typedef unsigned char WOLK_BOOL_T;

#define W_TRUE 1
#define W_FALSE 0

#define PAYLOAD_SIZE 256
#define TOPIC_SIZE 64
#define SERIAL_SIZE 32

#define READINGS_SIZE 5
#define READINGS_BUFFER_SIZE 192
#define READINGS_MQTT_SIZE 256

#define STR_64 64
#define STR_256 256

typedef struct _wolk_ctx_t wolk_ctx_t;

struct _wolk_ctx_t {
    char serial[SERIAL_SIZE];
    int sock;
    parser_t wolk_parser;
    wolk_queue actuator_queue;
    wolk_queue config_queue;
    reading_t readings[READINGS_SIZE];
    int readings_index;
    parser_type_t parser_type;
    PubSubClient *mqtt_client;
};

typedef enum {
    PROTOCOL_TYPE_WOLKSENSOR = 0,
    PROTOCOL_TYPE_JSON
} protocol_type_t;

typedef int (*send_func)(unsigned char *, unsigned int);
typedef int (*recv_func)(unsigned char *, unsigned int);

/** @brief Connect to WolkSense via mqtt
 *
 *  @param ctx library context
 *  @param snd_func function callback that will handle outgoing traffic
 *  @param rcv_func function callback that will handle incoming traffic
 *  @param device_key device key acquired through device registration on WolkSense
 *  @param password password acquired through device registration on WolkSense
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_connect (wolk_ctx_t *ctx, PubSubClient *client, const char *server, int port, const char *device_key, const char *password);

/** @brief Disconnect from WolkSense
 *
 *  @param ctx library context
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_disconnect (wolk_ctx_t *ctx);

/** @brief Choos between JSON and WolkSense protocol
 *
 *  If you set protocol PROTOCOL_TYPE_WOLKSENSOR then old protocol that has been used on WolkSensor is used.
 *  If PROTOCOL_TYPE_JSON is used then new JSON protocol is used.
 *
 *  @param ctx library context
 *  @param protocol protocol that will be used
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_set_protocol (wolk_ctx_t *ctx, protocol_type_t protocol);

/** @brief Set actuator references
 *
 *  If JSON protocol is used, then all actuator references needs to be set so that library can receive actuation messages from those actuators.
 *  If WolkSensor protocol is used, then this function is not used.
 *
 *  @param ctx library context
 *  @param num_of_items Number of references that are being set
 *  @param item actuator references
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_set_actuator_references (wolk_ctx_t *ctx, int num_of_items, const char *item, ...);

/** @brief Receive mqtt messages
 *
 *  Receiving mqtt messages on actuator topics.
 *  All messages are stored into queues and they are later used with functions wolk_read_actuator and wolk_read_config.
 *
 *  @param ctx library context
 *  @param timeout read timeout
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_receive (wolk_ctx_t *ctx);

/** @brief Extract actuation message from queue
 *
 *  Extract actuator message from queue. Messages is deleted after it is extracted.
 *
 *  @param ctx library context
 *  @param command Command received
 *  @param reference Reference received received
 *  @param value Value received
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_read_actuator (wolk_ctx_t *ctx, char *command, char *reference, char *value);

/** @brief Under contruction
 *
 *  @param ctx library context
 *  @param command Command received
 *  @param reference Reference received received
 *  @param value Value received
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_read_config (wolk_ctx_t *ctx, char *command, char *reference, char *value);

/** @brief Add string reading
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_string_reading(wolk_ctx_t *ctx,const char *reference,const char *value, uint32_t utc_time);

/** @brief Add numeric reading
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_numeric_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time);

/** @brief Add bool reading
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_bool_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time);

/** @brief Clear acumulated readings
 *
 *  @param ctx library context
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_clear_readings (wolk_ctx_t *ctx);

/** @brief Publish accumulated readings
 *
 *  @param ctx library context
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_publish (wolk_ctx_t *ctx);

/** @brief Publish single reading
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param type Parameter type. Available values are: DATA_TYPE_NUMERIC, DATA_TYPE_BOOLEAN, DATA_TYPE_STRING
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_publish_single (wolk_ctx_t *ctx,const char *reference,const char *value, data_type_t type, uint32_t utc_time);

/** @brief Publish Numeric actuator status
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param state Actuator state. Available states are: ACTUATOR_STATUS_READY, ACTUATOR_STATUS_BUSY, ACTUATOR_STATUS_ERROR
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_publish_num_actuator_status (wolk_ctx_t *ctx,const char *reference,double value, actuator_status_t state, uint32_t utc_time);

/** @brief Publish Boolean actuator status
 *
 *  @param ctx library context
 *  @param reference Parameter reference
 *  @param value Parameter value
 *  @param state Actuator state. Available states are: ACTUATOR_STATUS_READY, ACTUATOR_STATUS_BUSY, ACTUATOR_STATUS_ERROR
 *  @param utc_time Parameter UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_publish_bool_actuator_status (wolk_ctx_t *ctx,const char *reference,bool value, actuator_status_t state, uint32_t utc_time);

/** @brief Keep alive message
 *
 *  Function that needs to be called in main loop in order to keep connection alive
 *
 *  @param ctx library context
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_keep_alive (wolk_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif

