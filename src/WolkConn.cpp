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

#include "WolkConn.h"
#include "protocol/parser.h"
#include "MQTTClient.h"
#include "utility/wolk_utils.h"
#include "model/outbound_message.h"
#include "model/outbound_message_factory.h"

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define BOOL_FALSE          "false"
#define BOOL_TRUE           "true"

#define LASTWILL_TOPIC      "lastwill/"
#define LASTWILL_MESSAGE    "Gone offline"
#define QOS_LEVEL           1

#define PONG                "pong/"
#define EPOCH_WAIT          60000

const unsigned long ping_interval = 60000;

static WOLK_ERR_T _ping_keep_alive(wolk_ctx_t* ctx);

static WOLK_ERR_T subscribe_to(wolk_ctx_t* ctx, char* direction, char* message_type);
static WOLK_ERR_T receive(wolk_ctx_t* ctx);
static WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic);
static WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings);
static void callback(void *wolk, char* topic, byte* payload, unsigned int length); //TODO: questionable

static bool is_wolk_initialized(wolk_ctx_t* ctx);

static void handle_feeds(wolk_ctx_t* ctx, feed_t* feeds, size_t number_of_feeds);
//TODO
// static void handle_parameter_message(wolk_ctx_t* ctx, parameter_t* parameter_message, size_t number_of_parameters);
// static void handle_utc_command(wolk_ctx_t* ctx, utc_command_t* utc);
// static void handle_details_synchronization_message(wolk_ctx_t* ctx, feed_registration_t* feeds, size_t number_of_feeds,
//                                                    attribute_t* attributes, size_t number_of_attributes);
// static void handle_error_message(wolk_ctx_t* ctx, char* error);

static char *_double_to_string(double input, signed char width, unsigned char precision, char *output);

static char *_double_to_string(double input, signed char width, unsigned char precision, char *output) {
    sprintf(output, "%*.*f", width, precision, input);
    return output;
}

WOLK_ERR_T wolk_init(wolk_ctx_t* ctx,
                    const char* device_key, const char* device_password, PubSubClient *client, const char *server, int port,
                    outbound_mode_t outbound_mode, feed_handler_t feed_handler,
                    parameter_handler_t parameter_handler,
                    details_synchronization_handler_t details_synchronization_handler)
{
    /* Sanity check */

    WOLK_ASSERT(device_key != NULL);
    WOLK_ASSERT(device_password != NULL);

    WOLK_ASSERT(strlen(device_key) < DEVICE_KEY_SIZE);
    WOLK_ASSERT(strlen(device_password) < DEVICE_PASSWORD_SIZE);

    WOLK_ASSERT(protocol == PROTOCOL_SINGLE);

    ctx->mqtt_client = client;
    ctx->mqtt_client->setServer(server, port);
    ctx->mqtt_client->setCallback(callback);

    memset (ctx->device_key, 0, DEVICE_KEY_SIZE);
    strcpy (ctx->device_key, device_key);

    memset (ctx->device_password, 0, DEVICE_PASSWORD_SIZE);
    strcpy (ctx->device_password, device_password);

    ctx->feed_handler = feed_handler;
    ctx->outbound_mode = outbound_mode;

    initialize_parser(&ctx->parser, PARSER_TYPE);

    ctx->is_keep_alive_enabled = true;
    ctx->millis_last_ping = ping_interval;

    ctx->is_initialized = true;

    ctx->is_connected = false;

    return W_FALSE;
}

WOLK_ERR_T wolk_connect(wolk_ctx_t *ctx)
{
    /* Sanity check */
    WOLK_ASSERT(ctx->is_initialized == true);

    char lastwill_topic[TOPIC_SIZE];
    char client_id[TOPIC_SIZE];

    if (ctx->parser.type == PARSER_TYPE)
    {
        memset (lastwill_topic, 0, TOPIC_SIZE);
        strcpy (lastwill_topic, LASTWILL_TOPIC);
        strcat (lastwill_topic, ctx->device_key);
    }
    memset (client_id, 0, TOPIC_SIZE);
    sprintf(client_id,"%s%d",ctx->device_key,rand() % 1000);

    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (ctx->mqtt_client->connect(client_id, ctx->device_key, ctx->device_password, lastwill_topic, QOS_LEVEL, false, LASTWILL_MESSAGE))
    {
        Serial.println("connected!");
        ctx->is_connected = true;
    } 
    else 
    {
        Serial.print("failed with error code ");
        Serial.println(ctx->mqtt_client->state());
        ctx->is_connected = false;
        return W_TRUE;
    }

    /* Subscribe to topics */
    int result = subscribe_to(ctx, ctx->parser.P2D_TOPIC, ctx->parser.FEED_VALUES_MESSAGE_TOPIC);
    // TODO
    // subscribe_to(ctx, ctx->parser.SYNC_TIME_TOPIC);

    return result;
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

//TODO: check what to do with this
void callback(void *wolk, char* topic, byte*payload, unsigned int length)
{
    wolk_ctx_t *ctx = (wolk_ctx_t *)wolk;
    char payload_str[PAYLOAD_SIZE];
    memset (payload_str, 0, PAYLOAD_SIZE);

    memcpy(payload_str, payload, length);

    if (strstr(topic, PONG))
    {
        ctx->pong_received = true;
        uint32_t time;
        char value[10];
        parser_deserialize_pong(&ctx->parser, (char*)payload_str, (size_t)length, value);
        value[10] = 0;
        ctx->epoch_time = strtol(value, NULL, 10);
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


WOLK_ERR_T wolk_add_string_feed(wolk_ctx_t* ctx, const char* reference, wolk_string_feeds_t* feeds,
                                size_t number_of_feeds)
{
    /* Sanity check */
    WOLK_ASSERT(is_wolk_initialized(ctx));
    WOLK_ASSERT(is_wolk_initialized(reference));
    WOLK_ASSERT(is_wolk_initialized(feeds));
    WOLK_ASSERT(is_wolk_initialized(number_of_feeds));
    WOLK_ASSERT(number_of_feeds > FEEDS_MAX_NUMBER);

    feed_t feed;
    feed_initialize(&feed, number_of_feeds, reference);

    for (size_t i = 0; i < number_of_feeds; ++i) {
        if (feeds->utc_time < 1000000000000 && feeds->utc_time != 0) // Unit ms and zero is valid value
        {
            Serial.println("Failed UTC attached to feed. It has to be in ms!\n");
            return W_TRUE;
        }

        feed_set_data_at(&feed, feeds->value, i);
        feed_set_utc(&feed, feeds->utc_time);

        feeds++;
    }

    outbound_message_t outbound_message = {0};
    outbound_message_make_from_feeds(&ctx->parser, ctx->device_key, &feed, STRING, number_of_feeds, 1,
                                     &outbound_message);

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
}

WOLK_ERR_T wolk_add_numeric_feed(wolk_ctx_t* ctx, const char* reference, wolk_numeric_feeds_t* feeds,
                                 size_t number_of_feeds)
{
    /* Sanity check */
    WOLK_ASSERT(is_wolk_initialized(ctx));
    WOLK_ASSERT(is_wolk_initialized(reference));
    WOLK_ASSERT(is_wolk_initialized(feeds));
    WOLK_ASSERT(is_wolk_initialized(number_of_feeds));
    WOLK_ASSERT(number_of_feeds > FEEDS_MAX_NUMBER);

    char value_string[FEED_ELEMENT_SIZE] = "";
    feed_t feed;
    feed_initialize(&feed, number_of_feeds, reference);

    for (size_t i = 0; i < number_of_feeds; ++i) {
        if (feeds->utc_time < 1000000000000 && feeds->utc_time != 0) // Unit ms and zero is valid value
        {
            Serial.println("Failed UTC attached to feed. It has to be in ms!\n");
            return W_TRUE;
        }

        if (!snprintf(value_string, FEED_ELEMENT_SIZE, "%2f", feeds->value)) {
            return W_TRUE;
        }

        feed_set_data_at(&feed, value_string, i);
        feed_set_utc(&feed, feeds->utc_time);
        Serial.println(value_string);

        feeds++;
    }

    outbound_message_t outbound_message = {0};
    outbound_message_make_from_feeds(&ctx->parser, ctx->device_key, &feed, NUMERIC, number_of_feeds, 1, &outbound_message);

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
}

WOLK_ERR_T wolk_add_multi_value_numeric_feed(wolk_ctx_t* ctx, const char* reference, double* values,
                                             uint16_t value_size, uint64_t utc_time)
{
    /* Sanity check */
    WOLK_ASSERT(is_wolk_initialized(ctx));
    WOLK_ASSERT(value_size > FEEDS_MAX_NUMBER);

    if (utc_time < 1000000000000 && utc_time != 0) // Unit ms and zero is valid value
    {
        Serial.println("Failed UTC attached to feed. It has to be in ms!\n");
        return W_TRUE;
    }

    feed_t feed;
    feed_initialize(&feed, 1, reference); // one feed consisting of N numeric values
    feed_set_utc(&feed, utc_time);

    char value_string_representation[FEED_ELEMENT_SIZE] = "";
    for (size_t i = 0; i < value_size; ++i) {
        if (!snprintf(value_string_representation, FEED_ELEMENT_SIZE, "%f", values[i])) {
            return W_TRUE;
        }

        feed_set_data_at(&feed, value_string_representation, i);
    }

    outbound_message_t outbound_message = {0};
    outbound_message_make_from_feeds(&ctx->parser, ctx->device_key, &feed, VECTOR, 1, value_size, &outbound_message);

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
}

WOLK_ERR_T wolk_add_bool_feeds(wolk_ctx_t* ctx, const char* reference, wolk_boolean_feeds_t* feeds,
                               size_t number_of_feeds)
{
    /* Sanity check */
    WOLK_ASSERT(is_wolk_initialized(ctx));

    feed_t feed;
    feed_initialize(&feed, number_of_feeds, reference);

    for (size_t i = 0; i < number_of_feeds; ++i) {
        if (feeds->utc_time < 1000000000000 && feeds->utc_time != 0) // Unit ms and zero is valid value
        {
            Serial.println("Failed UTC attached to feed. It has to be in ms!\n");
            return W_TRUE;
        }

        feed_set_data_at(&feed, BOOL_TO_STR(feeds->value), i);
        feed_set_utc(&feed, feeds->utc_time);

        feeds++;
    }

    outbound_message_t outbound_message = {0};
    outbound_message_make_from_feeds(&ctx->parser, ctx->device_key, &feed, BOOLEAN, number_of_feeds, 1,
                                     &outbound_message);

    return persistence_push(&ctx->persistence, &outbound_message) ? W_FALSE : W_TRUE;
}

WOLK_ERR_T _publish (wolk_ctx_t *ctx, char *topic, char *readings)
{
    if(!(ctx->mqtt_client->publish(topic, readings)))
    {
        return W_TRUE;
    }
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

uint64_t wolk_request_timestamp(wolk_ctx_t* ctx)
{
    return ctx->epoch_time;
}

WOLK_ERR_T _subscribe (wolk_ctx_t *ctx, const char *topic)
{
    ctx->mqtt_client->subscribe(topic);
    return W_FALSE;
}

static WOLK_ERR_T subscribe_to(wolk_ctx_t* ctx, char* direction, char* message_type)
{
    char topic[TOPIC_SIZE] = "";

    if (ctx->parser.type == PARSER_TYPE)
    {
        memset(topic, '\0', TOPIC_SIZE);
        strcpy(&topic[0], direction);
        strcat(&topic[0], "/");
        strcat(&topic[0], ctx->device_key);
        strcat(&topic[0], "/");
        strcat(&topic[0], message_type);

        if (_subscribe(ctx, topic) != W_FALSE) 
        {
         return W_TRUE;
        }
    }
    Serial.println(topic);

    return W_FALSE;
}