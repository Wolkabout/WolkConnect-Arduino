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

#define ACTUATORS_STATUS_PATH "actuators/status/"
#define ACTUATORS_COMMANDS "actuators/commands/"

#define LASWILL_STRING "lastwill/"
#define GONE_OFFLINE "Gone offline"

#define SET_COMMAND "SET"
#define STATUS_COMMAND "STATUS"

static WOLK_ERR_T _wolk_subscribe (wolk_ctx_t *ctx, const char *topic);
static WOLK_ERR_T _wolk_set_parser (wolk_ctx_t *ctx, parser_type_t parser_type);
static WOLK_ERR_T _wolk_publish (wolk_ctx_t *ctx, char *topic, char *readings);
static void callback(void *wolk, char* topic, byte* payload, unsigned int length);

WOLK_ERR_T wolk_connect (wolk_ctx_t *ctx, PubSubClient *client, const char *server, int port, const char *device_key, const char *password)
{
    char lastwill_topic[TOPIC_SIZE];
    char sub_topic[TOPIC_SIZE];
    char client_id[TOPIC_SIZE];

    wolk_queue_init (&ctx->actuator_queue);
    wolk_queue_init (&ctx->config_queue);
    initialize_parser(&ctx->wolk_parser, ctx->parser_type);
    ctx->readings_index = 0;

    memset (lastwill_topic, 0, TOPIC_SIZE);
    strcpy (lastwill_topic, LASWILL_STRING);
    strcat (lastwill_topic, device_key);


    memset (ctx->serial, 0, SERIAL_SIZE);
    strcpy (ctx->serial, device_key);

    memset (client_id, 0, TOPIC_SIZE);
    sprintf(client_id,"%s%d",device_key,rand() % 1000);

    ctx->mqtt_client = client;


    ctx->mqtt_client->setServer(server, port);
    ctx->mqtt_client->setCallback(callback);

    ctx->mqtt_client->connect(client_id, ctx->serial, password);

    if (ctx->parser_type==PARSER_TYPE_MQTT)
    {
        memset (sub_topic, 0, TOPIC_SIZE);
        strcpy (sub_topic, CONFIG_PATH);
        strcat (sub_topic, device_key);
        if (_wolk_subscribe (ctx, sub_topic) != W_FALSE)
        {
            return W_TRUE;
        }
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_set_protocol (wolk_ctx_t *ctx, protocol_type_t protocol)
{
    if (protocol == PROTOCOL_TYPE_WOLKSENSOR)
    {
        _wolk_set_parser (ctx, PARSER_TYPE_MQTT);
    } else if (protocol == PROTOCOL_TYPE_JSON)
    {
        _wolk_set_parser (ctx, PARSER_TYPE_JSON);
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_set_actuator_references (wolk_ctx_t *ctx, int num_of_items,  const char *item, ...)
{
    va_list argptr;
    char pub_topic[TOPIC_SIZE];
    int i;
    if (ctx->parser_type == PARSER_TYPE_JSON)
    {
        va_start( argptr, item );

        const char* str = item;
        for (i=0;i<num_of_items;i++)
        {
            memset (pub_topic, 0, TOPIC_SIZE);

            strcpy(pub_topic,ACTUATORS_COMMANDS);
            strcat(pub_topic,ctx->serial);
            strcat(pub_topic,"/");
            strcat(pub_topic,str);


            if (_wolk_subscribe (ctx, pub_topic) != W_FALSE)
            {
                return W_TRUE;
            }

            str = va_arg( argptr, const char* );
        }
        va_end( argptr );
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
            case ACTUATOR_COMMAND_TYPE_STATUS:
            if (ctx->parser_type==PARSER_TYPE_MQTT)
            {
                wolk_queue_push(&ctx->actuator_queue, actuator_command_get_reference(command), STATUS_COMMAND, NON_EXISTING);
            } else if (ctx->parser_type==PARSER_TYPE_JSON)
            {
                wolk_queue_push(&ctx->actuator_queue, reference, STATUS_COMMAND, NON_EXISTING);
            }
            break;

            case ACTUATOR_COMMAND_TYPE_SET:
            if (ctx->parser_type==PARSER_TYPE_MQTT)
            {
                wolk_queue_push(&ctx->actuator_queue,  actuator_command_get_reference(command), SET_COMMAND, actuator_command_get_value(command));
            } else if (ctx->parser_type==PARSER_TYPE_JSON)
            {
                wolk_queue_push(&ctx->actuator_queue,  reference, SET_COMMAND, actuator_command_get_value(command));
            }
            break;

            case ACTUATOR_COMMAND_TYPE_UNKNOWN:
            break;
        }
    }

}


WOLK_ERR_T wolk_receive (wolk_ctx_t *ctx)
{
    if (ctx->mqtt_client->loop(ctx)==false)
    {
        return W_TRUE;
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_read_actuator (wolk_ctx_t *ctx, char *command, char *reference, char *value)
{
    if (wolk_queue_pop(&ctx->actuator_queue, reference, command, value) != Q_FALSE)
    {
        return W_TRUE;
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_read_config (wolk_ctx_t *ctx, char *command, char *reference, char *value)
{
    WOLK_UNUSED(ctx);
    WOLK_UNUSED(command);
    WOLK_UNUSED(reference);
    WOLK_UNUSED(value);
    return W_FALSE;
}

WOLK_ERR_T wolk_add_string_reading(wolk_ctx_t *ctx,const char *reference,const char *value, uint32_t utc_time)
{
    manifest_item_t string_sensor;
    manifest_item_init(&string_sensor,(char *) reference, READING_TYPE_SENSOR, DATA_TYPE_STRING);

    reading_init(&ctx->readings[ctx->readings_index], &string_sensor);
    reading_set_data(&ctx->readings[ctx->readings_index], (char *)value);
    reading_set_rtc(&ctx->readings[ctx->readings_index], utc_time);

    ctx->readings_index++;

    return W_FALSE;
}

WOLK_ERR_T wolk_add_numeric_reading(wolk_ctx_t *ctx,const char *reference,double value, uint32_t utc_time)
{
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

WOLK_ERR_T wolk_add_bool_reading(wolk_ctx_t *ctx,const char *reference,bool value, uint32_t utc_time)
{
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
    reading_clear_array(ctx->readings, ctx->readings_index);

    ctx->readings_index = 0;

    return W_FALSE;
}

WOLK_ERR_T wolk_publish (wolk_ctx_t *ctx)
{
    unsigned char buf[READINGS_MQTT_SIZE];
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    memset (buf, 0, READINGS_MQTT_SIZE);

    if (ctx->parser_type == PARSER_TYPE_MQTT )
    {

        char pub_topic[TOPIC_SIZE];
        memset (pub_topic, 0, TOPIC_SIZE);

        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->serial);

        size_t serialized_readings = parser_serialize_readings(&ctx->wolk_parser, &ctx->readings[0], ctx->readings_index, readings_buffer, READINGS_BUFFER_SIZE);

        wolk_clear_readings (ctx);



        if (_wolk_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
        {
            return W_TRUE;
        }

    } else if (ctx->parser_type == PARSER_TYPE_JSON)
    {
        int i=0;
        for (i=0; i<ctx->readings_index; i++)
        {

            char* value = reading_get_data(&ctx->readings[i]);

            if (wolk_publish_single (ctx,ctx->readings[i].manifest_item.reference,value, ctx->readings[i].manifest_item.data_type, ctx->readings[i].rtc) != W_FALSE)
            {
                return W_TRUE;
            }
        }

        wolk_clear_readings (ctx);


    }

    return W_FALSE;
}

WOLK_ERR_T wolk_publish_single (wolk_ctx_t *ctx,const char *reference,const char *value, data_type_t type, uint32_t utc_time)
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
        strcat(pub_topic,ctx->serial);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    } else if (ctx->parser_type == PARSER_TYPE_MQTT)
    {
        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->serial);
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
    if (_wolk_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
    {
        return W_TRUE;
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_publish_num_actuator_status (wolk_ctx_t *ctx,const char *reference,double value, actuator_status_t status, uint32_t utc_time)
{
    unsigned char buf[READINGS_MQTT_SIZE];
    int len;
    parser_t parser;
    reading_t readings;
    char readings_buffer[READINGS_BUFFER_SIZE];
    memset (readings_buffer, 0, READINGS_BUFFER_SIZE);
    memset (buf, 0, READINGS_MQTT_SIZE);
    initialize_parser(&parser, ctx->parser_type);

    char pub_topic[TOPIC_SIZE];
    memset (pub_topic, 0, TOPIC_SIZE);


    if (ctx->parser_type==PARSER_TYPE_JSON)
    {
        strcpy(pub_topic,ACTUATORS_STATUS_PATH);
        strcat(pub_topic,ctx->serial);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    } else if (ctx->parser_type==PARSER_TYPE_MQTT)
    {
        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->serial);
    }

    char value_str[STR_64];
    memset (value_str, 0, STR_64);
    dtostrf(value, 4, 2, value_str);

    manifest_item_t numeric_actuator;
    manifest_item_init(&numeric_actuator, (char *)reference, READING_TYPE_ACTUATOR, DATA_TYPE_NUMERIC);

    reading_init(&readings, &numeric_actuator);
    reading_set_rtc(&readings, utc_time);
    reading_set_data(&readings, value_str);
    reading_set_actuator_status(&readings, status);

    size_t serialized_readings = parser_serialize_readings(&parser, &readings, 1, readings_buffer, READINGS_BUFFER_SIZE);

    if (_wolk_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
    {
        return W_TRUE;
    }

    return W_FALSE;
}

WOLK_ERR_T wolk_publish_bool_actuator_status (wolk_ctx_t *ctx,const char *reference,bool value, actuator_status_t status, uint32_t utc_time)
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
        strcpy(pub_topic,ACTUATORS_STATUS_PATH);
        strcat(pub_topic,ctx->serial);
        strcat(pub_topic,"/");
        strcat(pub_topic,reference);
    } else if (ctx->parser_type == PARSER_TYPE_MQTT)
    {
        strcpy(pub_topic,SENSOR_PATH);
        strcat(pub_topic,ctx->serial);
    }

    manifest_item_t bool_actuator;
    manifest_item_init(&bool_actuator, (char *)reference, READING_TYPE_ACTUATOR, DATA_TYPE_BOOLEAN);

    reading_init(&readings, &bool_actuator);
    reading_set_rtc(&readings, utc_time);

    if (value == true)
    {
        reading_set_data(&readings, BOOL_TRUE);
    } else
    {
        reading_set_data(&readings, BOOL_FALSE);
    }
    reading_set_actuator_status(&readings, status);

    size_t serialized_readings = parser_serialize_readings(&parser, &readings, 1, readings_buffer, READINGS_BUFFER_SIZE);


    if (_wolk_publish (ctx, pub_topic, readings_buffer) != W_FALSE)
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
    ctx->mqtt_client->disconnect();
    return W_FALSE;
}

WOLK_ERR_T _wolk_publish (wolk_ctx_t *ctx, char *topic, char *readings)
{
    ctx->mqtt_client->publish(topic, readings);
    return W_FALSE;
}

WOLK_ERR_T _wolk_subscribe (wolk_ctx_t *ctx, const char *topic)
{
    ctx->mqtt_client->subscribe(topic);
    return W_FALSE;
}



WOLK_ERR_T _wolk_set_parser (wolk_ctx_t *ctx, parser_type_t parser_type)
{
    ctx->parser_type = parser_type;
    return W_FALSE;
}


