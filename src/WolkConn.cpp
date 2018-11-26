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

#include "WolkConn.h"
#include "utility/parser.h"
#include "MQTTClient.h"
#include "utility/wolk_utils.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define TIMEOUT_STEP 500000*2

#define NON_EXISTING "N/A"

#define BOOL_FALSE "false"
#define BOOL_TRUE "true"

#define KEEPALIVE_INTERVAL 60
#define CONFIG_PATH "config/"
#define SENSOR_PATH "sensors/"

#define RADINGS_PATH "readings/"

#define ACTUATORS_STATUS_TOPIC "actuators/status/"
#define ACTUATORS_COMMANDS_TOPIC_JSON "actuators/commands/"

#define CONFIGURATION_COMMANDS "configurations/commands/"

#define LASTWILL_TOPIC "lastwill/"
#define LASTWILL_MESSAGE "Gone offline"

#define SET_COMMAND "SET"
#define STATUS_COMMAND "STATUS"

static WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic);
static WOLK_ERR_T _parser_init (wolk_ctx_t *ctx, parser_type_t parser_type);
static WOLK_ERR_T _publish_single (wolk_ctx_t *ctx,const char *reference,const char *value, data_type_t type, uint32_t utc_time);
static WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings);
static void callback(void *wolk, char* topic, byte* payload, unsigned int length);

WOLK_ERR_T wolk_init(wolk_ctx_t* ctx, actuation_handler_t actuation_handler, actuator_status_provider_t actuator_status_provider,
                    const char* device_key, const char* device_password, PubSubClient *client, 
                    const char *server, int port, const char** actuator_references,
                    uint32_t num_actuator_references)
{
    /* Sanity check */

    WOLK_ASSERT(device_key != NULL);
    WOLK_ASSERT(device_password != NULL);

    WOLK_ASSERT(strlen(device_key) < DEVICE_KEY_SIZE);
    WOLK_ASSERT(strlen(device_password) < DEVICE_PASSWORD_SIZE);

    WOLK_ASSERT(protocol == PROTOCOL_TYPE_JSON);

    if (num_actuator_references > 0 && (actuation_handler == NULL || actuator_status_provider == NULL)) {
        WOLK_ASSERT(false);
        return W_TRUE;
    }

    _parser_init (ctx, PARSER_TYPE_JSON);

    initialize_parser(&ctx->wolk_parser, ctx->parser_type);
    ctx->readings_index = 0;

    ctx->mqtt_client = client;
    ctx->mqtt_client->setServer(server, port);
    ctx->mqtt_client->setCallback(callback);

    memset (ctx->device_key, 0, DEVICE_KEY_SIZE);
    strcpy (ctx->device_key, device_key);

    memset (ctx->device_password, 0, DEVICE_PASSWORD_SIZE);
    strcpy (ctx->device_password, device_password);

    ctx->actuator_references = actuator_references;
    ctx->num_actuator_references = num_actuator_references;

    ctx->actuation_handler = actuation_handler;
    ctx->actuator_status_provider = actuator_status_provider;

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

    memset (lastwill_topic, 0, TOPIC_SIZE);
    strcpy (lastwill_topic, LASTWILL_TOPIC);
    strcat (lastwill_topic, ctx->device_key);

    memset (client_id, 0, TOPIC_SIZE);
    sprintf(client_id,"%s%d",ctx->device_key,rand() % 1000);

    ctx->mqtt_client->connect(client_id, ctx->device_key, ctx->device_password);

    if (ctx->parser_type==PARSER_TYPE_MQTT)
    {
        memset (sub_topic, 0, TOPIC_SIZE);
        strcpy (sub_topic, CONFIG_PATH);
        strcat (sub_topic, ctx->device_key);
        if (_subscribe (ctx, sub_topic) != W_FALSE)
        {   
            return W_TRUE;
        }
    }

    char pub_topic[TOPIC_SIZE];
    int i;
    if (ctx->parser_type == PARSER_TYPE_JSON)
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
    }

    for (i = 0; i < ctx->num_actuator_references; ++i) {
        const char* reference = ctx->actuator_references[i];

        wolk_publish_actuator_status(ctx, reference);

    }

    return W_FALSE;
}


void callback(void *wolk, char* topic, byte* payload, unsigned int length) {

    int i=0;
    actuator_command_t commands_buffer[128];
    wolk_ctx_t *ctx = (wolk_ctx_t *)wolk;
    char reference[STR_64];
    char payload_str[STR_256];
    memset (reference, 0, STR_64);
    memset (payload_str, 0, STR_256);

    memcpy(payload_str, payload, length);

    if (ctx->parser_type == PARSER_TYPE_JSON)
    {
        char *start_ptr = strrchr(topic, '/');
        if (start_ptr != NULL)
        {

            strncpy(reference, start_ptr+1, strlen(topic)-(start_ptr-topic)-1);
        }
    }

    size_t num_deserialized_commands = parser_deserialize_commands(&ctx->wolk_parser, payload_str, length, &commands_buffer[0], 128);

    for (i = 0; i < num_deserialized_commands; ++i) {
        actuator_command_t* command = &commands_buffer[i];

        switch(actuator_command_get_type(command))
        {
            case ACTUATOR_COMMAND_TYPE_SET:
                if(ctx->actuation_handler != NULL)
                {
                        ctx->actuation_handler(reference, actuator_command_get_value(command));
                }

        /* Fallthrough */
        /* break; */
            case ACTUATOR_COMMAND_TYPE_STATUS:
                if(ctx->actuator_status_provider != NULL)
                {
                    wolk_publish_actuator_status(ctx, reference);
                }

            break;

            case ACTUATOR_COMMAND_TYPE_UNKNOWN:
            break;
        }
    }

}


WOLK_ERR_T wolk_process (wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    if (ctx->mqtt_client->loop(ctx)==false)
    {
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

    reading_init(&ctx->readings[ctx->readings_index], &string_sensor);
    reading_set_data(&ctx->readings[ctx->readings_index], (char *)value);
    reading_set_rtc(&ctx->readings[ctx->readings_index], utc_time);

    ctx->readings_index++;

    return W_FALSE;
}

WOLK_ERR_T wolk_add_numeric_sensor_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    char value_str[STR_64];
    memset (value_str, 0, STR_64);
    dtostrf(value, 4, 2, value_str);

    reading_init(&ctx->readings[ctx->readings_index], &numeric_sensor);
    reading_set_data(&ctx->readings[ctx->readings_index], value_str);
    reading_set_rtc(&ctx->readings[ctx->readings_index], utc_time);

    ctx->readings_index++;

    return W_FALSE;
}

WOLK_ERR_T wolk_add_bool_sensor_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    manifest_item_t bool_sensor;
    manifest_item_init(&bool_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_BOOLEAN);


    reading_init(&ctx->readings[ctx->readings_index], &bool_sensor);
    if (value == true)
    {
        reading_set_data(&ctx->readings[ctx->readings_index], BOOL_TRUE);
    } else
    {
        reading_set_data(&ctx->readings[ctx->readings_index], BOOL_FALSE);
    }
    reading_set_rtc(&ctx->readings[ctx->readings_index], utc_time);

    ctx->readings_index++;

    return W_FALSE;
}

WOLK_ERR_T wolk_clear_readings (wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    reading_clear_array(ctx->readings, ctx->readings_index);

    ctx->readings_index = 0;

    return W_FALSE;
}

WOLK_ERR_T wolk_publish (wolk_ctx_t *ctx)
{

    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    unsigned char buf[READINGS_MQTT_SIZE];
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    memset (buf, 0, READINGS_MQTT_SIZE);

    if (ctx->parser_type == PARSER_TYPE_MQTT )
    {

        char pub_topic[TOPIC_SIZE];
        memset (pub_topic, 0, TOPIC_SIZE);

        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->device_key);

        size_t serialized_readings = parser_serialize_readings(&ctx->wolk_parser, &ctx->readings[0], ctx->readings_index, readings_buffer, READINGS_BUFFER_SIZE);

        wolk_clear_readings (ctx);



        if (_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
        {
            return W_TRUE;
        }

    } else if (ctx->parser_type == PARSER_TYPE_JSON)
    {
        int i=0;
        for (i=0; i<ctx->readings_index; i++)
        {

            char* value = reading_get_data(&ctx->readings[i]);

            if (_publish_single (ctx,ctx->readings[i].manifest_item.reference,value, ctx->readings[i].manifest_item.data_type, ctx->readings[i].rtc) != W_FALSE)
            {
                return W_TRUE;
            }
        }

        wolk_clear_readings (ctx);


    }

    return W_FALSE;
}

WOLK_ERR_T _publish_single (wolk_ctx_t *ctx,const char *reference,const char *value, data_type_t type, uint32_t utc_time)
{
    unsigned char buf[READINGS_MQTT_SIZE];
    parser_t parser;
    reading_t readings;
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    memset (buf, 0, READINGS_MQTT_SIZE);
    initialize_parser(&parser, ctx->parser_type);

    char pub_topic[TOPIC_SIZE];
    memset (pub_topic, 0, TOPIC_SIZE);


    if (ctx->parser_type == PARSER_TYPE_JSON)
    {
        strcpy(pub_topic,RADINGS_PATH);
        strcat(pub_topic,ctx->device_key);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    } else if (ctx->parser_type == PARSER_TYPE_MQTT)
    {
        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->device_key);
    }

    if (type==DATA_TYPE_STRING)
    {
        manifest_item_t string_sensor;
        manifest_item_init(&string_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_STRING);

        reading_init(&readings, &string_sensor);
        reading_set_data(&readings, (char *)value);

    } else if (type==DATA_TYPE_BOOLEAN)
    {
        manifest_item_t bool_sensor;
        manifest_item_init(&bool_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_BOOLEAN);
        reading_init(&readings, &bool_sensor);
        reading_set_data(&readings, (char *)value);
    } else if (type==DATA_TYPE_NUMERIC)
    {
        manifest_item_t numeric_sensor;
        manifest_item_init(&numeric_sensor, (char *)reference, READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);
        reading_init(&readings, &numeric_sensor);
        reading_set_data(&readings, (char *)value);
    }

    reading_set_rtc(&readings, utc_time);

    parser_serialize_readings(&parser, &readings, 1, readings_buffer, READINGS_BUFFER_SIZE);
    if (_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
    {
        return W_TRUE;
    }

    return W_FALSE;
}


WOLK_ERR_T wolk_publish_actuator_status (wolk_ctx_t *ctx,const char *reference)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    unsigned char buf[READINGS_MQTT_SIZE];
    parser_t parser;
    reading_t readings;
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    memset (buf, 0, READINGS_MQTT_SIZE);
    initialize_parser(&parser, ctx->parser_type);

    char pub_topic[TOPIC_SIZE];
    memset (pub_topic, 0, TOPIC_SIZE);


    actuator_status_t actuator_status = ctx->actuator_status_provider(reference);
    

    if (ctx->parser_type==PARSER_TYPE_JSON)
    {
        strcpy(pub_topic,ACTUATORS_STATUS_TOPIC);
        strcat(pub_topic,ctx->device_key);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    } else if (ctx->parser_type==PARSER_TYPE_MQTT)
    {
        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->device_key);
    }

    manifest_item_t manifest_item;
    manifest_item_init(&manifest_item, reference, READING_TYPE_ACTUATOR, DATA_TYPE_STRING);

    reading_init(&readings, &manifest_item);
    reading_set_rtc(&readings, 0);
    reading_set_data(&readings, actuator_status_get_value(&actuator_status));
    reading_set_actuator_state(&readings, actuator_status_get_state(&actuator_status));

    size_t serialized_readings = parser_serialize_readings(&parser, &readings, 1, readings_buffer, READINGS_BUFFER_SIZE);

    if (_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
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
    strcpy (lastwill_topic, LASTWILL_TOPIC);
    strcat (lastwill_topic, ctx->device_key);

    ctx->mqtt_client->publish(lastwill_topic, LASTWILL_MESSAGE);
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


WOLK_ERR_T _parser_init (wolk_ctx_t *ctx, parser_type_t parser_type)
{
    ctx->parser_type = parser_type;
    return W_FALSE;
}


