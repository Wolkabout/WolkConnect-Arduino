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

#include "utility/actuator_command.h"
#include "WolkConn.h"
#include "utility/parser.h"
#include "MQTTClient.h"
#include "utility/wolk_utils.h"
#include "utility/outbound_message.h"
#include "utility/outbound_message_factory.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BOOL_FALSE "false"
#define BOOL_TRUE "true"

#define READINGS_PATH_JSON "readings/"

#define ACTUATORS_STATUS_TOPIC_JSON "actuators/status/"
#define ACTUATORS_COMMANDS_TOPIC_JSON "actuators/commands/"

#define LASTWILL_TOPIC_JSON "lastwill/"
#define LASTWILL_MESSAGE_JSON "Gone offline"

/*ping keep alive every 60 seconds*/
#define PING_KEEP_ALIVE_INTERVAL 300

static const char* CONFIGURATION_COMMANDS_TOPIC_JSON = "configurations/commands/";

static WOLK_ERR_T _ping_keep_alive(wolk_ctx_t* ctx, uint32_t tick);

static WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic);
static void _parser_init(wolk_ctx_t* ctx, protocol_t protocol);
static WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings);
static void callback(void *wolk, char* topic, byte* payload, unsigned int length);

static void _handle_actuator_command(wolk_ctx_t* ctx, actuator_command_t* actuator_command);
static void _handle_configuration_command(wolk_ctx_t* ctx, configuration_command_t* configuration_command);

WOLK_ERR_T wolk_init(wolk_ctx_t* ctx, actuation_handler_t actuation_handler, actuator_status_provider_t actuator_status_provider,
                    configuration_handler_t configuration_handler, configuration_provider_t configuration_provider,
                    const char* device_key, const char* device_password, PubSubClient *client, 
                    const char *server, int port, protocol_t protocol, const char** actuator_references,
                    uint32_t num_actuator_references)
{
    /* Sanity check */

    WOLK_ASSERT(device_key != NULL);
    WOLK_ASSERT(device_password != NULL);

    WOLK_ASSERT(strlen(device_key) < DEVICE_KEY_SIZE);
    WOLK_ASSERT(strlen(device_password) < DEVICE_PASSWORD_SIZE);

    WOLK_ASSERT(protocol == PROTOCOL_JSON_SINGLE);

    if (num_actuator_references > 0 && (actuation_handler == NULL || actuator_status_provider == NULL)) {
        WOLK_ASSERT(false);
        return W_TRUE;
    }
    if ((configuration_handler != NULL && configuration_provider == NULL)
        || (configuration_handler == NULL && configuration_provider != NULL)) {
        WOLK_ASSERT(false);
        return W_TRUE;
    }

    ctx->mqtt_client = client;
    ctx->mqtt_client->setServer(server, port);
    ctx->mqtt_client->setCallback(callback);

    memset (ctx->device_key, 0, DEVICE_KEY_SIZE);
    strcpy (ctx->device_key, device_key);

    memset (ctx->device_password, 0, DEVICE_PASSWORD_SIZE);
    strcpy (ctx->device_password, device_password);

    ctx->actuation_handler = actuation_handler;
    ctx->actuator_status_provider = actuator_status_provider;

    ctx->configuration_handler = configuration_handler;
    ctx->configuration_provider = configuration_provider;

    ctx->actuator_references = actuator_references;
    ctx->num_actuator_references = num_actuator_references;

    ctx->protocol = protocol;
    _parser_init (ctx, protocol);

    ctx->is_keep_alive_enabled = true;
    ctx->milliseconds_since_last_ping_keep_alive = PING_KEEP_ALIVE_INTERVAL;

    ctx->is_initialized = true;

    return W_FALSE;

}

WOLK_ERR_T wolk_connect (wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    char lastwill_topic[TOPIC_SIZE];
    char sub_topic[TOPIC_SIZE];
    char client_id[TOPIC_SIZE];

    if (ctx->parser.type == PARSER_TYPE_JSON)
    {
        memset (lastwill_topic, 0, TOPIC_SIZE);
        strcpy (lastwill_topic, LASTWILL_TOPIC_JSON);
        strcat (lastwill_topic, ctx->device_key);
    }
    memset (client_id, 0, TOPIC_SIZE);
    sprintf(client_id,"%s%d",ctx->device_key,rand() % 1000);

    while (!ctx->mqtt_client->connected()) 
    {
        Serial.print("Attempting MQTT connection...");
        // Attempt to connect
        if (ctx->mqtt_client->connect(client_id, ctx->device_key, ctx->device_password, lastwill_topic, NULL, NULL, LASTWILL_MESSAGE_JSON)) 
        {
            Serial.println("connected");
        } 
        else 
        {
            Serial.print("failed, rc=");
            Serial.print(ctx->mqtt_client->state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }

    char pub_topic[TOPIC_SIZE];
    char topic_buf[TOPIC_SIZE];
    int i;
    if (ctx->parser.type == PARSER_TYPE_JSON)
    {
        for (i = 0; i < ctx->num_actuator_references ; i++)
        {
            const char* str = ctx->actuator_references[i];
            memset (pub_topic, 0, TOPIC_SIZE);

            strcpy(pub_topic,ACTUATORS_COMMANDS_TOPIC_JSON);
            strcat(pub_topic,ctx->device_key);
            strcat(pub_topic,"/");
            strcat(pub_topic,str);

            if (_subscribe (ctx, pub_topic) != W_FALSE)
            {
                return W_TRUE;
            }
        }
        memset(topic_buf, '\0', TOPIC_SIZE);
        strcpy(&topic_buf[0], CONFIGURATION_COMMANDS_TOPIC_JSON);
        strcat(&topic_buf[0], ctx->device_key);

        if (_subscribe(ctx, topic_buf) != W_FALSE) 
        {
         return W_TRUE;
        }
    }  

    for (i = 0; i < ctx->num_actuator_references; ++i) 
    {
        const char* reference = ctx->actuator_references[i];

        wolk_publish_actuator_status(ctx, reference);
    }

    configuration_command_t configuration_command;
    configuration_command_init(&configuration_command, CONFIGURATION_COMMAND_TYPE_CURRENT);
    _handle_configuration_command(ctx, &configuration_command);

    return W_FALSE;
}

void callback(void *wolk, char* topic, byte*payload, unsigned int length)
{
    wolk_ctx_t *ctx = (wolk_ctx_t *)wolk;
    char payload_str[PAYLOAD_SIZE];
    memset (payload_str, 0, PAYLOAD_SIZE);

    memcpy(payload_str, payload, length);

    if (strstr(topic, ACTUATORS_COMMANDS_TOPIC_JSON) != NULL)
    {
        actuator_command_t actuator_command;
        const size_t num_deserialized_commands = parser_deserialize_actuator_commands(&ctx->parser, topic, strlen(topic), (char*)payload_str, (size_t)length, &actuator_command, 1);
        if (num_deserialized_commands != 0) 
        {
            _handle_actuator_command(ctx, &actuator_command);
        }
        
    }
    else if (strstr(topic, CONFIGURATION_COMMANDS_TOPIC_JSON)) 
    {
        configuration_command_t configuration_command;
        const size_t num_deserialized_commands = parser_deserialize_configuration_commands(&ctx->parser, (char*)payload_str, (size_t)length, &configuration_command, 1);
        
        if (num_deserialized_commands != 0) 
        {
            _handle_configuration_command(ctx, &configuration_command);
        }
    }
}

static void _handle_configuration_command(wolk_ctx_t* ctx, configuration_command_t* configuration_command)
{
    switch (configuration_command_get_type(configuration_command)) {
    case CONFIGURATION_COMMAND_TYPE_SET:
        if (ctx->configuration_handler != NULL) {
            ctx->configuration_handler(configuration_command_get_references(configuration_command),
                                       configuration_command_get_values(configuration_command),
                                       configuration_command_get_number_of_items(configuration_command));
        }

        /* Fallthrough */
        /* break; */

    case CONFIGURATION_COMMAND_TYPE_CURRENT:
        if (ctx->configuration_provider != NULL) {
            char references[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_REFERENCE_SIZE];
            char values[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_VALUE_SIZE];

            const size_t num_configuration_items =
                ctx->configuration_provider(&references[0], &values[0], CONFIGURATION_ITEMS_SIZE);
            if (num_configuration_items == 0) {
                return;
            }

            outbound_message_t outbound_message;
            if (!outbound_message_make_from_configuration(&ctx->parser, ctx->device_key, references, values,
                                                          num_configuration_items, &outbound_message)) {
                return;
            }

            _publish(ctx, outbound_message.topic, outbound_message.payload);
                
        }
        break;

    case CONFIGURATION_COMMAND_TYPE_UNKNOWN:
        break;
    }
}

static void _handle_actuator_command(wolk_ctx_t* ctx, actuator_command_t* command)
{
    switch(actuator_command_get_type(command))
    {
        case ACTUATOR_COMMAND_TYPE_SET:
        if(ctx->actuation_handler != NULL)
            {
                ctx->actuation_handler(actuator_command_get_reference(command), actuator_command_get_value(command));
            }

        /* Fallthrough */
        /* break; */
        case ACTUATOR_COMMAND_TYPE_STATUS:
            if(ctx->actuator_status_provider != NULL)
            {
                wolk_publish_actuator_status(ctx, actuator_command_get_reference(command));
            }

        break;

        case ACTUATOR_COMMAND_TYPE_UNKNOWN:
        break;
        }
}

WOLK_ERR_T wolk_process (wolk_ctx_t *ctx, uint32_t tick)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    if (ctx->mqtt_client->loop(ctx)==false)
    {
        Serial.println("Disconnected, attempting to reconnect");
        wolk_connect(ctx);
        return W_TRUE;
    }

    if (_ping_keep_alive(ctx, tick) != W_FALSE) {
        Serial.println("Ping keep alive failed");
        return W_TRUE;
    }

    return W_FALSE;
}


WOLK_ERR_T wolk_add_string_sensor_reading(wolk_ctx_t *ctx,const char *reference,const char *value, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t string_sensor;
    manifest_item_init(&string_sensor,(char *) reference, READING_TYPE_SENSOR, DATA_TYPE_STRING);

    reading_t reading;
    reading_init(&reading, &string_sensor);
    reading_set_data(&reading, value);
    reading_set_rtc(&reading, utc_time);

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}

WOLK_ERR_T wolk_add_multi_value_string_sensor_reading(wolk_ctx_t* ctx, const char* reference,
                                                      const char (*values)[READING_SIZE], uint16_t values_size,
                                                      uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);
    WOLK_ASSERT(READING_DIMENSIONS > 1);

    manifest_item_t string_sensor;
    manifest_item_init(&string_sensor, reference, READING_TYPE_SENSOR, DATA_TYPE_STRING);
    manifest_item_set_reading_dimensions_and_delimiter(&string_sensor, values_size, DATA_DELIMITER);

    reading_t reading;
    reading_init(&reading, &string_sensor);
    reading_set_rtc(&reading, utc_time);

    for (uint32_t i = 0; i < values_size; ++i) {
        reading_set_data_at(&reading, values[i], i);
    }

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}

WOLK_ERR_T wolk_add_numeric_sensor_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    char value_str[PAYLOAD_SIZE];
    memset (value_str, 0, PAYLOAD_SIZE);
    dtostrf(value, 4, 2, value_str);

    reading_t reading;
    reading_init(&reading, &numeric_sensor);
    reading_set_data(&reading, value_str);
    reading_set_rtc(&reading, utc_time);

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);


    return W_FALSE;
}

WOLK_ERR_T wolk_add_multi_value_numeric_sensor_reading(wolk_ctx_t* ctx, const char* reference, double* values,
                                                       uint16_t values_size, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);
    WOLK_ASSERT(READING_DIMENSIONS > 1);

    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, reference, READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);
    manifest_item_set_reading_dimensions_and_delimiter(&numeric_sensor, values_size, DATA_DELIMITER);
    
    reading_t reading;
    reading_init(&reading, &numeric_sensor);
    reading_set_rtc(&reading, utc_time);

    for(uint32_t i = 0; i < values_size; i++)
    {
        char value_str[PAYLOAD_SIZE];
        memset (value_str, 0, PAYLOAD_SIZE);
        dtostrf(values[i], 4, 2, value_str);

        reading_set_data_at(&reading, value_str, i);
    }

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}


WOLK_ERR_T wolk_add_bool_sensor_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t bool_sensor;
    manifest_item_init(&bool_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_BOOLEAN);

    reading_t reading;
    reading_init(&reading, &bool_sensor);
    if (value == true)
    {
        reading_set_data(&reading, BOOL_TRUE);
    } else
    {
        reading_set_data(&reading, BOOL_FALSE);
    }
    reading_set_rtc(&reading, utc_time);

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}

WOLK_ERR_T wolk_add_multi_value_bool_sensor_reading(wolk_ctx_t* ctx, const char* reference, bool* values,
                                                    uint16_t values_size, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);
    WOLK_ASSERT(READING_DIMENSIONS > 1);

    manifest_item_t string_sensor;
    manifest_item_init(&string_sensor, reference, READING_TYPE_SENSOR, DATA_TYPE_STRING);
    manifest_item_set_reading_dimensions_and_delimiter(&string_sensor, values_size, DATA_DELIMITER);

    reading_t reading;
    reading_init(&reading, &string_sensor);
    reading_set_rtc(&reading, utc_time);

    for (uint32_t i = 0; i < values_size; ++i) {
        reading_set_data_at(&reading, BOOL_TO_STR(values[i]), i);
    }

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}

WOLK_ERR_T wolk_add_alarm(wolk_ctx_t* ctx, const char* reference, bool state, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t alarm;
    manifest_item_init(&alarm, reference, READING_TYPE_ALARM, DATA_TYPE_STRING);

    reading_t alarm_reading;
    reading_init(&alarm_reading, &alarm);
    reading_set_rtc(&alarm_reading, utc_time);
    reading_set_data(&alarm_reading, (state==true ? "ON" : "OFF"));

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&ctx->parser, ctx->device_key, &alarm_reading, 1, &outbound_message);

    _publish(ctx, outbound_message.topic, outbound_message.payload);

    return W_FALSE;
}

WOLK_ERR_T wolk_publish_actuator_status (wolk_ctx_t *ctx,const char *reference)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    parser_t parser;
    reading_t readings;
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    initialize_parser(&parser, ctx->parser.type);

    char pub_topic[TOPIC_SIZE];
    memset (pub_topic, 0, TOPIC_SIZE);


    actuator_status_t actuator_status = ctx->actuator_status_provider(reference);
    

    if (ctx->parser.type==PARSER_TYPE_JSON)
    {
        strcpy(pub_topic,ACTUATORS_STATUS_TOPIC_JSON);
        strcat(pub_topic,ctx->device_key);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    }

    manifest_item_t manifest_item;
    manifest_item_init(&manifest_item, reference, READING_TYPE_ACTUATOR, DATA_TYPE_STRING);

    reading_init(&readings, &manifest_item);
    reading_set_rtc(&readings, 0);
    reading_set_data(&readings, actuator_status_get_value(&actuator_status));
    reading_set_actuator_state(&readings, actuator_status_get_state(&actuator_status));

    size_t serialized_readings = parser_serialize_readings(&parser, &readings, 1, readings_buffer, READINGS_BUFFER_SIZE);

    outbound_message_t outbound_message;

    outbound_message_init(&outbound_message, pub_topic, readings_buffer);

    if (_publish (ctx, outbound_message.topic, outbound_message.payload) != W_FALSE)
    {
        return W_TRUE;
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_keep_alive (wolk_ctx_t *ctx)
{
    WOLK_UNUSED(ctx);
    return W_FALSE;
}


WOLK_ERR_T wolk_disconnect(wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    char lastwill_topic[TOPIC_SIZE];

    memset (lastwill_topic, 0, TOPIC_SIZE);
    strcpy (lastwill_topic, LASTWILL_TOPIC_JSON);
    strcat (lastwill_topic, ctx->device_key);

    ctx->mqtt_client->publish(lastwill_topic, LASTWILL_MESSAGE_JSON);
    ctx->mqtt_client->disconnect();
    return W_FALSE;
}

WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings)
{
    ctx->mqtt_client->publish(topic, readings);
    return W_FALSE;
}

WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic)
{
    ctx->mqtt_client->subscribe(topic);
    return W_FALSE;
}

WOLK_ERR_T wolk_disable_keep_alive(wolk_ctx_t* ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx);

    ctx->is_keep_alive_enabled = false;
    return W_FALSE;
}

static WOLK_ERR_T _ping_keep_alive(wolk_ctx_t* ctx, uint32_t tick)
{
    if (!ctx->is_keep_alive_enabled) {
        return W_FALSE;
    }

    if (ctx->milliseconds_since_last_ping_keep_alive < PING_KEEP_ALIVE_INTERVAL) {
        ctx->milliseconds_since_last_ping_keep_alive += tick;
        return W_FALSE;
    }

    outbound_message_t outbound_message;
    outbound_message_make_from_keep_alive_message(&ctx->parser, ctx->device_key, &outbound_message);

    if (_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE) {
        return W_TRUE;
    }


    ctx->milliseconds_since_last_ping_keep_alive = 0;
    return W_FALSE;
}

static void _parser_init(wolk_ctx_t* ctx, protocol_t protocol)
{
    switch (protocol) {
    case PROTOCOL_JSON_SINGLE:
        initialize_parser(&ctx->parser, PARSER_TYPE_JSON);
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }
}

