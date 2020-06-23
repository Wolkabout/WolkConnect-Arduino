#include "gtest/gtest.h"

#include "outbound_message_factory.h"
#include "outbound_message.h"
#include "parser.h"
#include "reading.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>


TEST(outboundMessageMakeFromReadings, OneReading)
{
    parser_t parser;
    initialize_parser(&parser, PARSER_TYPE);
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "T", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);

    reading_t reading;
    reading_init(&reading, &numeric_sensor);
    reading_set_data(&reading, "23.4");
    reading_set_rtc(&reading, 0);

    outbound_message_t outbound_message;
    outbound_message_make_from_readings(&parser, "xjlsmn6q89cm030r", &reading, 1, &outbound_message);
    EXPECT_STREQ(outbound_message_get_topic(&outbound_message), "d2p/sensor_reading/d/xjlsmn6q89cm030r/r/T");
}
/*
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}*/
