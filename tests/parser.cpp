#include "gtest/gtest.h"

#include "outbound_message_factory.h"
#include "outbound_message.h"
#include "parser.h"
#include "reading.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>


TEST(OutboundMessage, OutboundMessageMakeFromReadings)
{
    parser_t parser;
    initialize_parser(&parser, PARSER_TYPE);
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "T", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    reading_t reading;
    reading_init(&reading, &numeric_sensor);
    reading_set_data(&reading, "23.4");
    reading_set_rtc(&reading, 1592911456);

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&parser, "xjlsmn6q89cm030r", &reading, 1, &outbound_message);
    EXPECT_STREQ(outbound_message_get_topic(&outbound_message), "d2p/sensor_reading/d/xjlsmn6q89cm030r/r/T");
    EXPECT_STREQ(outbound_message_get_payload(&outbound_message), "{\"utc\":1592911456,\"data\":\"23.4\"}");
}

TEST(OutboundMessage, OutboundMessageMakeFromActuatorStatus)
{
    parser_t parser;
    initialize_parser(&parser, PARSER_TYPE);
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "T", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    actuator_status_t actuator_status;
    actuator_status.value[0] = '2';
    actuator_status.value[1] = '4';
    actuator_status.value[2] = '\0';
    actuator_status.state = ACTUATOR_STATE_READY;

    outbound_message_t outbound_message;
    outbound_message_make_from_actuator_status(&parser, "xjlsmn6q89cm030r", &actuator_status, "A", &outbound_message);
    EXPECT_STREQ(outbound_message_get_topic(&outbound_message), "d2p/actuator_status/d/xjlsmn6q89cm030r/r/A");
    EXPECT_STREQ(outbound_message_get_payload(&outbound_message), "{\"status\":\"READY\",\"value\":\"24\"}");
}

TEST(OutboundMessage, OutboundMessageMakeFromConfiguration)
{
    parser_t parser;
    initialize_parser(&parser, PARSER_TYPE);
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "T", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    char references[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_REFERENCE_SIZE] = {{"A11"}, {"A12"}};
    char values[CONFIGURATION_ITEMS_SIZE][CONFIGURATION_VALUE_SIZE] = {{"50"}, {"60"}};

    outbound_message_t outbound_message;
    outbound_message_make_from_configuration(&parser, "xjlsmn6q89cm030r", references, values, 2, &outbound_message);
    EXPECT_STREQ(outbound_message_get_topic(&outbound_message), "d2p/configuration_get/d/xjlsmn6q89cm030r");
    EXPECT_STREQ(outbound_message_get_payload(&outbound_message), "{\"values\":{\"A11\":\"50\",\"A12\":\"60\"}}");
}

TEST(OutboundMessage, OutboundMessageMakeFromKeepAliveMessage)
{
    parser_t parser;
    initialize_parser(&parser, PARSER_TYPE);
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "T", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    outbound_message_t outbound_message;
    outbound_message_make_from_keep_alive_message(&parser, "xjlsmn6q89cm030r", &outbound_message);
    EXPECT_STREQ(outbound_message_get_topic(&outbound_message), "ping/xjlsmn6q89cm030r");
    EXPECT_STREQ(outbound_message_get_payload(&outbound_message), "");
}
/*
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}*/
