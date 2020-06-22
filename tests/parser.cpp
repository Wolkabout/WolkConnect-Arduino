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
    EXPECT_EQ(outbound_message_get_topic(&outbound_message), "d2p/sensor_reading/d/xjlsmn6q89cm030r/");
}
