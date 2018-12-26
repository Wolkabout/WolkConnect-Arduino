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

#ifndef WOLK_H
#define WOLK_H

#include "MQTTClient.h"
#include "utility/WolkQueue.h"
#include "utility/parser.h"
#include "utility/size_definitions.h"
#include "utility/actuator_status.h"
#include "utility/outbound_message.h"
#include "utility/outbound_message_factory.h"
#include "utility/dtostrf_fix.h"
#include "utility/wolk_utils.h"

//#include "utility/circular_buffer.h"
#include "utility/in_memory_persistence.h"

#include "Arduino.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FW_VERSION_MAJOR 2 // number 0 -99
#define FW_VERSION_MINOR 1 // number 0 -99
#define FW_VERSION_PATCH 0 // number 0 -99

/**
 * @brief Supported protocols, WolkConnect libararies currently support only PROTOCOL_JSON_SINGLE
 */
typedef enum { PROTOCOL_JSON_SINGLE = 0 } protocol_t;
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
 * @brief Declaration of actuator handler.
 * Actuator reference and value are the pairs of data on the same place in own arrays.
 *
 * @param reference actuator references defined in manifest on WolkAbout IoT Platform.
 * @param value value received from WolkAbout IoT Platform.
 */
typedef void (*actuation_handler_t)(const char* reference, const char* value);
/**
 * @brief Declaration of actuator status
 *
 * @param reference actuator references define in manifest on WolkAbout IoT Platform
 */
typedef actuator_status_t (*actuator_status_provider_t)(const char* reference);

/**
 * @brief Declaration of configuration handler.
 * Configuration reference and value are the pairs of data on the same place in own arrays.
 *
 * @param reference actuator references define in manifest on WolkAbout IoT Platform
 * @param value actuator values received from WolkAbout IoT Platform.
 * @param num_configuration_items number of used configuration parameters
 */
typedef void (*configuration_handler_t)(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                        char (*value)[CONFIGURATION_VALUE_SIZE], size_t num_configuration_items);
/**
 * @brief Declaration of configuration provider
 *
 * @param reference configuration references define in manifest on WolkAbout IoT Platform
 * @param value configuration values received from WolkAbout IoT Platform
 * @param num_configuration_items number of used configuration parameters
 */
typedef size_t (*configuration_provider_t)(char (*reference)[CONFIGURATION_REFERENCE_SIZE],
                                           char (*value)[CONFIGURATION_VALUE_SIZE], size_t max_num_configuration_items);
/**
 * @brief  WolkAbout IoT Platform connector context.
 * Most of the parameters are used to initialize WolkConnect library forwarding to wolk_init().
 */

typedef struct _wolk_ctx_t wolk_ctx_t;

struct _wolk_ctx_t {
    int sock;
    PubSubClient *mqtt_client;

    actuation_handler_t actuation_handler;                  /**< Callback for handling received actuation from WolkAbout IoT Platform. @see actuation_handler_t*/
    actuator_status_provider_t actuator_status_provider;    /**< Callback for providing the current actuator status to WolkAbout IoT Platform. @see actuator_status_provider_t*/

    configuration_handler_t configuration_handler;          /**< Callback for handling received configuration from WolkAbout IoT Platform. @see configuration_handler_t*/
    configuration_provider_t configuration_provider;        /**< Callback for providing the current configuration status to WolkAbout IoT Platform. @see configuration_provider_t*/

    char device_key[DEVICE_KEY_SIZE];                       /**<  Authentication parameters for WolkAbout IoT Platform. Obtained as a result of device creation on the platform.*/
    char device_password[DEVICE_PASSWORD_SIZE];             /**<  Authentication parameters for WolkAbout IoT Platform. Obtained as a result of device creation on the platform.*/

    protocol_t protocol;                               /**<  Used protocol for communication with WolkAbout IoT Platform. @see protocol_type_t*/
    parser_t parser;

    const char** actuator_references;
    uint32_t num_actuator_references;

    bool is_keep_alive_enabled;
    uint32_t milliseconds_since_last_ping_keep_alive;

    bool is_initialized;

    //outbound_message_t outbound_messages[STORE_SIZE];
    //int number_of_msgs;

    //circular_buffer_t buffer;
};

void wolk_init_in_memory_persistence(wolk_ctx_t* ctx, void* storage, uint32_t size, bool wrap);
/**
 * @brief Initializes WolkAbout IoT Platform connector context
 * @param ctx Context
 *
 * @param actuation_handler function pointer to 'actuation_handler_t' implementation
 * @param actuator_status_provider function pointer to 'actuator_status_provider_t' implementation
 *
 * @param configuration_handler function pointer to 'configuration_handler_t' implementation
 * @param configuration_provider function pointer to 'configuration_provider_t' implementation
 *
 * @param device_key Device key provided by WolkAbout IoT Platform upon device
 * creation
 * @param password Device password provided by WolkAbout IoT Platform device
 * upon device creation
 * @param client MQQT Client
 * @param server MQQT Server
 * @param port Port to connect to
 * @param protocol Protocol specified for device
 * @param actuator_references Array of strings containing references of
 * actuators that device possess
 * @param num_actuator_references Number of actuator references contained in
 * actuator_references
 * 
 * @return Error code
 */
WOLK_ERR_T wolk_init(wolk_ctx_t* ctx, actuation_handler_t actuation_handler, actuator_status_provider_t actuator_status_provider,
                    configuration_handler_t configuration_handler, configuration_provider_t configuration_provider,
                    const char* device_key, const char* device_password, PubSubClient *client, 
                    const char *server, int port, protocol_t protocol, const char** actuator_references,
                    uint32_t num_actuator_references);

/**
 * @brief Connect to WolkAbout IoT Platform
 *
 * Prior to connecting, following must be performed:
 *  - Context must be initialized via wolk_init()
 *
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
 * @brief Must be called periodically to keep alive connection to WolkAbout IoT
 * platform, obtain and perform actuation requests
 *
 * @param ctx Context
 * @param tick Period at which wolk_process is called
 *
 * @return Error code
 */
WOLK_ERR_T wolk_process (wolk_ctx_t *ctx, uint32_t tick);

/** @brief Add string reading
 *
 *  @param ctx library context
 *  @param reference Sensor reference
 *  @param value Sensor value
 *  @param utc_time UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_string_sensor_reading(wolk_ctx_t *ctx,const char *reference,const char *value, uint32_t utc_time);

/** @brief Add multi-value string reading
 *
 *  @param ctx Context
 *  @param reference Sensor reference
 *  @param values Sensor values
 *  @param values_size Number of sensor dimensions
 *  @param utc_time UTC time of sensor value acquisition [seconds]
 *
 *  @return Error code
 */
WOLK_ERR_T wolk_add_multi_value_string_sensor_reading(wolk_ctx_t* ctx, const char* reference,
                                                      const char (*values)[READING_SIZE], uint16_t values_size,
                                                      uint32_t utc_time);
/** @brief Add numeric reading
 *
 *  @param ctx library context
 *  @param reference Sensor reference
 *  @param value Sensor value
 *  @param utc_time UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_numeric_sensor_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time);

/**
 * @brief Add multi-value numeric reading
 *
 * @param ctx Context
 * @param reference Sensor reference
 * @param values Sensor values
 * @param values_size Number of sensor dimensions
 * @param utc_time UTC time of sensor value acquisition [seconds]
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_multi_value_numeric_sensor_reading(wolk_ctx_t* ctx, const char* reference, double* values,
                                                       uint16_t values_size, uint32_t utc_time);
/** @brief Add bool reading
 *
 *  @param ctx library context
 *  @param reference Sensor reference
 *  @param value Sensor value
 *  @param utc_time UTC time. If unable to retrieve UTC set 0
 *  @return Error value is returned
 */
WOLK_ERR_T wolk_add_bool_sensor_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time);

/**
 * @brief Add multi-value bool reading
 *
 * @param ctx Context
 * @param reference Sensor reference
 * @param values Sensor values
 * @param values_size Number of sensor dimensions
 * @param utc_time UTC time of sensor value acquisition [seconds]
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_multi_value_bool_sensor_reading(wolk_ctx_t* ctx, const char* reference, bool* values,
                                                    uint16_t values_size, uint32_t utc_time);

/**
 * @brief Add alarm
 *
 * @param ctx Context
 * @param reference Alarm reference
 * @param message Alarm message
 * @param utc_time UTC time when alarm was risen [seconds]
 *
 * @return Error code
 */
WOLK_ERR_T wolk_add_alarm(wolk_ctx_t* ctx, const char* reference, bool state, uint32_t utc_time);

/**
 * @brief Obtains actuator status via actuator_status_provider_t and publishes
 * it.
 *
 * @param ctx Context
 * @param reference Actuator reference
 *
 * @return Error code
 */
WOLK_ERR_T wolk_publish_actuator_status (wolk_ctx_t *ctx,const char *reference);
 
 /**
 *@brief Obratins configuration via configuration_provider and publishes it.
 *
 *@param ctx Context
 *
 *@return Error code
 */
WOLK_ERR_T wolk_publish_configuration(wolk_ctx_t* ctx);

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


#ifdef __cplusplus
}
#endif

#endif

