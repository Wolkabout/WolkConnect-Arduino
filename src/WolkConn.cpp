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

#define READINGS_PATH "readings/"

#define ACTUATORS_STATUS_TOPIC "d2p/actuator_status/"
#define ACTUATORS_SET_TOPIC "p2d/actuator_set/"

#define LASTWILL_TOPIC "lastwill/"
#define LASTWILL_MESSAGE "Gone offline"

#define PONG "pong/"
#define EPOCH_WAIT 60000

const unsigned long ping_interval = 60000;

static const char* CONFIGURATION_SET_TOPIC = "p2d/configuration_set/";

static WOLK_ERR_T _ping_keep_alive(wolk_ctx_t* ctx);

static WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic);
static void _parser_init(wolk_ctx_t* ctx, protocol_t protocol);
static WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings);
static void callback(void *wolk, char* topic, byte* payload, unsigned int length);

static void _handle_actuator_command(wolk_ctx_t* ctx, actuator_command_t* actuator_command);
static void _handle_configuration_command(wolk_ctx_t* ctx, configuration_command_t* configuration_command);

WOLK_ERR_T wolk_init_in_memory_persistence(wolk_ctx_t* ctx, void* storage, uint32_t size, bool wrap)
{
    in_memory_persistence_init(storage, size, wrap);
    persistence_init(&ctx->persistence, in_memory_persistence_push, in_memory_persistence_peek,
                     in_memory_persistence_pop, in_memory_persistence_is_empty);
    return W_FALSE;
}

WOLK_ERR_T wolk_init_custom_persistence(wolk_ctx_t* ctx, persistence_push_t push, persistence_peek_t peek,
                                        persistence_pop_t pop, persistence_is_empty_t is_empty)
{
    persistence_init(&ctx->persistence, push, peek, pop, is_empty);

    return W_FALSE;
}

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

    WOLK_ASSERT(protocol == PROTOCOL_SINGLE);

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
    ctx->millis_last_ping = ping_interval;

    ctx->is_initialized = true;

    ctx->is_connected = false;

    return W_FALSE;

}

WOLK_ERR_T wolk_connect (wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    char lastwill_topic[TOPIC_SIZE];
    char sub_topic[TOPIC_SIZE];
    char client_id[TOPIC_SIZE];

    if (ctx->parser.type == PARSER_TYPE)
    {
        memset (lastwill_topic, 0, TOPIC_SIZE);
        strcpy (lastwill_topic, LASTWILL_TOPIC);
        strcat (lastwill_topic, ctx->device_key);
    }
    memset (client_id, 0, TOPIC_SIZE);
    sprintf(client_id,"%s%d",ctx->device_key,rand() % 1000);

    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (ctx->mqtt_client->connect(client_id, ctx->device_key, ctx->device_password, lastwill_topic, NULL, NULL, LASTWILL_MESSAGE))
    {
        Serial.println("connected!");
        ctx->is_connected = true;
    } 
    else 
    {
        Serial.print("failed with error code ");
        Serial.println(ctx->mqtt_client->state());
        ctx->is_connected = false;
    }

    char pub_topic[TOPIC_SIZE];
    char topic_buf[TOPIC_SIZE];
    int i;
    if (ctx->parser.type == PARSER_TYPE)
    {
        for (i = 0; i < ctx->num_actuator_references ; i++)
        {
            const char* str = ctx->actuator_references[i];
            memset (pub_topic, 0, TOPIC_SIZE);

            strcpy(pub_topic,ACTUATORS_SET_TOPIC);
            strcat(pub_topic,"d/");
            strcat(pub_topic,ctx->device_key);
            strcat(pub_topic,"/r/");
            strcat(pub_topic,str);

            if (_subscribe (ctx, pub_topic) != W_FALSE)
            {
                return W_TRUE;
            }
        }
        memset(topic_buf, '\0', TOPIC_SIZE);
        strcpy(&topic_buf[0], CONFIGURATION_SET_TOPIC);
        strcat(&topic_buf[0], "d/");
        strcat(&topic_buf[0], ctx->device_key);

        if (_subscribe(ctx, topic_buf) != W_FALSE) 
        {
         return W_TRUE;
        }

        memset(topic_buf, '\0', TOPIC_SIZE);
        strcpy(&topic_buf[0], PONG);
        strcat(&topic_buf[0], "d/");
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
    configuration_command_init(&configuration_command);
    _handle_configuration_command(ctx, &configuration_command);

    return W_FALSE;
}

void callback(void *wolk, char* topic, byte*payload, unsigned int length)
{
    wolk_ctx_t *ctx = (wolk_ctx_t *)wolk;
    char payload_str[PAYLOAD_SIZE];
    memset (payload_str, 0, PAYLOAD_SIZE);

    memcpy(payload_str, payload, length);

    if (strstr(topic, ACTUATORS_SET_TOPIC) != NULL)
    {
        actuator_command_t actuator_command;
        const size_t num_deserialized_commands = parser_deserialize_actuator_commands(&ctx->parser, topic, strlen(topic), (char*)payload_str, (size_t)length, &actuator_command, 1);
        if (num_deserialized_commands != 0) 
        {
            _handle_actuator_command(ctx, &actuator_command);
        }
        
    }
    else if (strstr(topic, CONFIGURATION_SET_TOPIC))
    {
        configuration_command_t configuration_command;
        const size_t num_deserialized_commands = parser_deserialize_configuration_commands(&ctx->parser, (char*)payload_str, (size_t)length, &configuration_command, 1);
        
        if (num_deserialized_commands != 0) 
        {
            _handle_configuration_command(ctx, &configuration_command);
        }
    }
    else if (strstr(topic, PONG))
    {
        ctx->pong_received = true;
        uint32_t time;
        char value[10];
        parser_deserialize_pong(&ctx->parser, (char*)payload_str, (size_t)length, value);
        value[10] = 0;
        ctx->epoch_time = strtol(value, NULL, 10);
    }
}

static void _handle_configuration_command(wolk_ctx_t* ctx, configuration_command_t* configuration_command)
{
    if (ctx->configuration_handler != NULL)
    {
        ctx->configuration_handler(configuration_command_get_references(configuration_command),
                                   configuration_command_get_values(configuration_command),
                                   configuration_command_get_number_of_items(configuration_command));
    }
}

static void _handle_actuator_command(wolk_ctx_t* ctx, actuator_command_t* command)
{

    if(ctx->actuation_handler != NULL)
    {
        ctx->actuation_handler(actuator_command_get_reference(command), actuator_command_get_value(command));
    }
}

WOLK_ERR_T wolk_process (wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    if (ctx->mqtt_client->loop(ctx)==false)
    {
        Serial.println("MQTT lost connection!");
        Serial.print("Client state:");
        Serial.println(ctx->mqtt_client->state());
        ctx->is_connected = false;
        return W_TRUE;
    }

    if (_ping_keep_alive(ctx) != W_FALSE) {
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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;

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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;

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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;

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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;

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

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;

}

WOLK_ERR_T wolk_publish_actuator_status(wolk_ctx_t* ctx, const char* reference)
{
    /* Sanity check */
    WOLK_ASSERT(_is_wolk_initialized(ctx));

    if (ctx->actuator_status_provider != NULL) {
        actuator_status_t actuator_status = ctx->actuator_status_provider(reference);

        outbound_message_t outbound_message;
        if (!outbound_message_make_from_actuator_status(&ctx->parser, ctx->device_key, &actuator_status, reference,
                                                        &outbound_message)) {
            return W_TRUE;
        }
        if (_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE) {
            return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
        }
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_publish_configuration(wolk_ctx_t* ctx)
{
    /* Sanity check */
    WOLK_ASSERT(_is_wolk_initialized(ctx));

    if (ctx->configuration_provider != NULL) 
        {
            char references[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_REFERENCE_SIZE];
            char values[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_VALUE_SIZE];

            const size_t num_configuration_items =
                ctx->configuration_provider(&references[0], &values[0], CONFIGURATION_ITEMS_SIZE);
            if (num_configuration_items == 0) 
            {
                return W_TRUE;
            }

            outbound_message_t outbound_message;
            if (!outbound_message_make_from_configuration(&ctx->parser, ctx->device_key, references, values,
                                                          num_configuration_items, &outbound_message)) 
            {
                return W_TRUE;
            }

            if(_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE)
            {
                return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
            }

            return W_FALSE;
        }
        return W_TRUE;
}

WOLK_ERR_T wolk_disconnect(wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    char lastwill_topic[TOPIC_SIZE];

    memset (lastwill_topic, 0, TOPIC_SIZE);
    strcpy (lastwill_topic, LASTWILL_TOPIC);
    strcat (lastwill_topic, ctx->device_key);

    ctx->mqtt_client->publish(lastwill_topic, LASTWILL_MESSAGE);
    ctx->mqtt_client->disconnect();
    ctx->is_connected = false;
    return W_FALSE;
}

WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings)
{
    if(!(ctx->mqtt_client->publish(topic, readings)))
    {
        return W_TRUE;
    }
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

static WOLK_ERR_T _ping_keep_alive(wolk_ctx_t* ctx)
{

    if (!ctx->is_keep_alive_enabled) {
        return W_FALSE;
    }

    unsigned long currentMillis = millis();
 
    if(currentMillis - ctx->millis_last_ping > ping_interval) {

        ctx->millis_last_ping = currentMillis;

        outbound_message_t outbound_message;
        outbound_message_make_from_keep_alive_message(&ctx->parser, ctx->device_key, &outbound_message);
    
        if (_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE) {
            return W_TRUE;
        }
        ctx->pong_received = false;

        return W_FALSE;
    }

    return W_FALSE;
}

static void _parser_init(wolk_ctx_t* ctx, protocol_t protocol)
{
    switch (protocol) {
    case PROTOCOL_WOLKABOUT:
        initialize_parser(&ctx->parser, PARSER_TYPE);
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }
}

WOLK_ERR_T wolk_publish(wolk_ctx_t* ctx)
{
    outbound_message_t outbound_message;
    uint16_t batch_size = 50;
    uint8_t i;
    for(i = 0; i < batch_size; i++)
    {
        if (persistence_is_empty(&ctx->persistence))
        {
            break;
        }
        if (!persistence_peek(&ctx->persistence, &outbound_message)) {
            continue;
        }

        if (_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE) {
            Serial.println("Publish failed!");
            return W_TRUE;
        }

        persistence_pop(&ctx->persistence, &outbound_message);
    }
    Serial.println("Published data!");
    return W_FALSE;
}

WOLK_ERR_T wolk_update_epoch(wolk_ctx_t* ctx)
{
    WOLK_ASSERT(ctx->is_connected == true);

    ctx->pong_received = false;

    outbound_message_t outbound_message;

    outbound_message_make_from_keep_alive_message(&ctx->parser, ctx->device_key, &outbound_message);

    if (_publish(ctx, outbound_message.topic, outbound_message.payload) != W_FALSE) {
        return W_TRUE;
    }

    delay(100);

    unsigned long currentMillis = millis();

    while (millis() - currentMillis < EPOCH_WAIT) {
        wolk_process(ctx);
        digitalWrite(LED_BUILTIN, HIGH);
        if(ctx->pong_received){
            digitalWrite(LED_BUILTIN, LOW);
            wolk_disconnect(ctx);
            return W_FALSE;
        }
    }

    Serial.println("Epoch time not received");

    return W_TRUE;
}
