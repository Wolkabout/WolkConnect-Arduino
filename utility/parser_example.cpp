#include "wolk_utils.h"
#include "manifest_item.h"
#include "reading.h"
#include "parser.h"

#include <stdio.h>
#include <string.h>

void print_actuator_commands(actuator_command_t* first_command, size_t num_of_commands);

void print_config_items_commands(configuration_item_command_t* first_command, size_t num_of_commands);

void parser_example(void)
{
    size_t i;
    size_t serialized_readings;
    size_t num_deserialized_commands;
    
    parser_t parser;
    
    initialize_parser(&parser, PARSER_TYPE_MQTT);
    
    /* Sensor(s) */
    manifest_item_t numeric_sensor;
    manifest_item_init(&numeric_sensor, "NUMERIC_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);
    
    manifest_item_t bool_sensor;
    manifest_item_init(&bool_sensor, "BOOLEAN_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_BOOLEAN);
    
    manifest_item_t string_sensor;
    manifest_item_init(&string_sensor, "STRING_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_STRING);
    
    manifest_item_t delimited_numeric_sensor;
    manifest_item_init(&delimited_numeric_sensor, "DELIMITED_NUMERIC_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_NUMERIC);
    manifest_item_set_reading_dimensions_and_delimiter(&delimited_numeric_sensor, 3, "#");
    
    manifest_item_t delimited_bool_sensor;
    manifest_item_init(&delimited_bool_sensor, "DELIMITED_BOOLEAN_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_BOOLEAN);
    manifest_item_set_reading_dimensions_and_delimiter(&delimited_bool_sensor, 3, "~");
    
    manifest_item_t delimited_string_sensor;
    manifest_item_init(&delimited_string_sensor, "DELIMITED_STRING_REFERENCE", READING_TYPE_SENSOR, DATA_TYPE_STRING);
    manifest_item_set_reading_dimensions_and_delimiter(&delimited_string_sensor, 3, "_");
    
    /* Actuator(s) */
    manifest_item_t numeric_actuator;
    manifest_item_init(&numeric_actuator, "NUMERIC_ACTUATOR_REFERENCE", READING_TYPE_ACTUATOR, DATA_TYPE_NUMERIC);
    
    manifest_item_t bool_actuator;
    manifest_item_init(&bool_actuator, "BOOL_ACTUATOR_REFERENCE", READING_TYPE_ACTUATOR, DATA_TYPE_BOOLEAN);
    
    manifest_item_t string_actuator;
    manifest_item_init(&string_actuator, "STRING_ACTUATOR_REFERENCE", READING_TYPE_ACTUATOR, DATA_TYPE_STRING);
    
    manifest_item_t delimited_actuator;
    manifest_item_init(&delimited_actuator, "DELIMITED_ACTUATOR_REFERENCE", READING_TYPE_ACTUATOR, DATA_TYPE_STRING);
    manifest_item_set_reading_dimensions_and_delimiter(&delimited_actuator, 3, "&");
    
    /* Sensor reading(s) */
    uint16_t readings_buffer_size = 1024;
    char readings_buffer[1024];
    
    reading_t readings[10];
    reading_init(&readings[0], &numeric_sensor);
    reading_set_data(&readings[0], "12.3");
    reading_set_rtc(&readings[0], 123456789);
    
    reading_init(&readings[1], &bool_sensor);
    reading_set_data(&readings[1], "false");
    /* reading_set_rtc(&readings[1], 456789123); */
    
    reading_init(&readings[2], &string_sensor);
    reading_set_data(&readings[2], "string-data");
    reading_set_rtc(&readings[2], 798123456);
    
    reading_init(&readings[3], &delimited_numeric_sensor);
    reading_set_data_at(&readings[3], "23.4", 0);
    reading_set_data_at(&readings[3], "34.4", 1);
    reading_set_data_at(&readings[3], "45.6", 2);
    
    reading_init(&readings[4], &delimited_bool_sensor);
    reading_set_data_at(&readings[4], "false", 0);
    reading_set_data_at(&readings[4], "true", 1);
    reading_set_data_at(&readings[4], "false", 2);
    reading_set_rtc(&readings[4], 456456456);
    
    reading_init(&readings[5], &delimited_string_sensor);
    reading_set_data_at(&readings[5], "one", 0);
    reading_set_data_at(&readings[5], "two", 1);
    reading_set_data_at(&readings[5], "three", 2);
    reading_set_rtc(&readings[5], 789789789);
    
    /* Actuator reading(s) */
    reading_init(&readings[6], &numeric_actuator);
    reading_set_rtc(&readings[6], 789789789);
    reading_set_data(&readings[6], "66785.4");
    reading_set_actuator_status(&readings[6], ACTUATOR_STATUS_BUSY);
    
    reading_init(&readings[7], &bool_actuator);
    reading_set_rtc(&readings[7], 987654321);
    reading_set_data(&readings[7], "false");
    reading_set_actuator_status(&readings[7], ACTUATOR_STATUS_ERROR);
    
    reading_init(&readings[8], &string_actuator);
    reading_set_rtc(&readings[8], 1111222333);
    reading_set_data(&readings[8], "string_actuator_value");
    reading_set_actuator_status(&readings[8], ACTUATOR_STATUS_READY);
    
    reading_init(&readings[9], &delimited_actuator);
    reading_set_rtc(&readings[9], 979835312);
    reading_set_data_at(&readings[9], "uno", 0);
    reading_set_data_at(&readings[9], "due", 1);
    reading_set_data_at(&readings[9], "tre", 2);
    reading_set_actuator_status(&readings[9], ACTUATOR_STATUS_READY);
    
    /* Serialize reading(s) */
    serialized_readings = parser_serialize_readings(&parser, &readings[0], 10, readings_buffer, readings_buffer_size);
    
    printf("%lu reading(s) serialized to WolkSense format:\n%s\n\n", serialized_readings, readings_buffer);
    
    /* Deserialize command(s) */
    char received_wolksense_commands_buffer[] = "STATUS SLIDER;SET actuator_reference:1123354.123;ASD;SET ref;N_COM ref;SET SWITCH:true;";
    actuator_command_t commands_buffer[128];
    
    num_deserialized_commands = parser_deserialize_commands(&parser, received_wolksense_commands_buffer, WOLK_ARRAY_LENGTH(received_wolksense_commands_buffer), &commands_buffer[0], 128);
    
    printf("%lu command(s) deserialized:", num_deserialized_commands);
    print_actuator_commands(commands_buffer, num_deserialized_commands);
    
    initialize_parser(&parser, PARSER_TYPE_JSON);
    
    /* Serialize reading(s) */
    printf("\n\nSerializing 10 reading(s) to JSON format:\n");
    for (i = 0; i < 10; ++i) {
        serialized_readings = parser_serialize_readings(&parser, &readings[i], 1, readings_buffer, readings_buffer_size);
        printf("%s\n", readings_buffer);
    }
    
    /* Deserialize command(s) */
    char status_command[] = "{\"command\":\"STATUS\"}";
    num_deserialized_commands = parser_deserialize_commands(&parser, status_command, sizeof(status_command), &commands_buffer[0], 1);
    
    char status_command_two[] = "{\"command\":\"SET\",\"value\":\"desired_value\"}";
    num_deserialized_commands += parser_deserialize_commands(&parser, status_command_two, sizeof(status_command_two), &commands_buffer[1], 1);
    
    char status_command_three[] = "{\"command\":\"NEW_COM\",\"value\":\"new_command_desired_value\"}";
    num_deserialized_commands += parser_deserialize_commands(&parser, status_command_three, sizeof(status_command_three), &commands_buffer[2], 1);
    
    printf("\n%lu command(s) deserialized:\n", num_deserialized_commands);
    print_actuator_commands(commands_buffer, num_deserialized_commands);
    
    /* Serialize configuration item(s) */
    configuration_item_t configuration_items[3];
    configuration_item_init(&configuration_items[0], "conf-item-one-name", "conf-item-one-value");
    configuration_item_init(&configuration_items[1], "conf-item-two-name", "conf-item-two-value");
    configuration_item_init(&configuration_items[2], "conf-item-three-name", "conf-item-three-value");
    
    size_t num_serialize_configuration_items = parser_serialize_configuration_items(&parser, configuration_items, 3, readings_buffer, readings_buffer_size);
    printf("\n\nSerialized %lu configuration items to JSON format:\n%s\n", num_serialize_configuration_items, readings_buffer);
    
    /* Deserialize configuration item(s) */
    char received_set_config_items_buffer[] = "{\"command\":\"SET\",\"values\":{\"config_item_one\":\"value_one\",\"config_item_two\":\"value_two\",\"config_item_three\":\"value_three\"}}";
    configuration_item_command_t received_config_commands[5];
    size_t num_deserialized_config_commands = parser_deserialize_configuration_items(&parser, received_set_config_items_buffer, WOLK_ARRAY_LENGTH(received_set_config_items_buffer), received_config_commands, 3);
    
    printf("\nDeserialized %lu configuration items from JSON format:\n", num_deserialized_config_commands);
    print_config_items_commands(received_config_commands, num_deserialized_config_commands);
    
    char received_current_config_items_buffer[] = "{\"command\":\"CURRENT\"}";
    num_deserialized_config_commands = parser_deserialize_configuration_items(&parser, received_current_config_items_buffer, WOLK_ARRAY_LENGTH(received_current_config_items_buffer), received_config_commands, 2);
    
    printf("\nDeserialized %lu configuration items from JSON format:\n", num_deserialized_config_commands);
    print_config_items_commands(received_config_commands, num_deserialized_config_commands);
}

void print_actuator_commands(actuator_command_t* first_command, size_t num_of_commands)
{
    size_t i;
    actuator_command_t* command = first_command;
    
    for (i = 0; i < num_of_commands; ++i, ++command) {
        printf("Command type: ");
        switch(actuator_command_get_type(command))
        {
            case ACTUATOR_COMMAND_TYPE_STATUS:
                printf("STATUS\n");
                printf("  Reference:  %s\n", actuator_command_get_reference(command));
                break;
                
            case ACTUATOR_COMMAND_TYPE_SET:
                printf("SET\n");
                printf("  Reference:  %s\n", actuator_command_get_reference(command));
                printf("  Value:      %s\n", actuator_command_get_value(command));
                break;
                
            case ACTUATOR_COMMAND_TYPE_UNKNOWN:
                printf("Unknown command\n");
                printf("  Reference:  %s\n", actuator_command_get_reference(command));
                printf("  Value:      %s\n", actuator_command_get_value(command));
                break;
                
            default:
                continue;
        }
    }
}

void print_config_items_commands(configuration_item_command_t* first_command, size_t num_of_commands)
{
    size_t i;
    configuration_item_command_t* command = first_command;
    
    for (i = 0; i < num_of_commands; ++i, ++command) {
        printf("Command type: ");
        switch(configuration_item_command_get_type(command))
        {
            case CONFIG_ITEM_COMMAND_TYPE_STATUS:
                printf("STATUS\n");
                break;
                
            case CONFIG_ITEM_COMMAND_TYPE_SET:
                printf("SET\n");
                printf("  Name:       %s\n", configuration_item_command_get_name(command));
                printf("  Value:      %s\n", configuration_item_command_get_value(command));
                break;
                
            case CONFIG_ITEM_COMMAND_TYPE_UNKNOWN:
                printf("Unknown command\n");
                printf("  Name:       %s\n", configuration_item_command_get_name(command));
                printf("  Value:      %s\n", configuration_item_command_get_value(command));
                break;
                
            default:
                continue;
        }
    }
}
