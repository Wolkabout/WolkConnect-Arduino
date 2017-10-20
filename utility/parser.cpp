#include "parser.h"
#include "configuration_item.h"
#include "mqtt_parser.h"
#include "json_parser.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void initialize_parser(parser_t* parser, parser_type_t parser_type)
{
    parser->type = parser_type;

    switch(parser->type)
    {
    case PARSER_TYPE_MQTT:
        parser->serialize_readings = mqtt_serialize_readings;
        parser->deserialize_commands = mqtt_deserialize_commands;
        parser->serialize_configuration_items = mqtt_serialize_configuration_items;
        parser->deserialize_configuration_items = mqtt_deserialize_configuration_items;
        break;

    case PARSER_TYPE_JSON:
        parser->serialize_readings = json_serialize_readings;
        parser->deserialize_commands = json_deserialize_commands;
        parser->serialize_configuration_items = json_serialize_configuration_items;
        parser->deserialize_configuration_items = json_deserialize_configuration_items;
        break;

    default:
        /* Sanity check */
        WOLK_ASSERT(false);
    }
}

size_t parser_serialize_readings(parser_t* parser, reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size)
{
    return parser->serialize_readings(first_reading, num_readings, buffer, buffer_size);
}

size_t parser_deserialize_commands(parser_t* parser, char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size)
{
    WOLK_UNUSED(buffer_size);
    return parser->deserialize_commands(buffer, buffer_size, commands_buffer, commands_buffer_size);
}

size_t parser_serialize_configuration_items(parser_t* parser, configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size)
{
    return parser->serialize_configuration_items(first_config_item, num_config_items, buffer, buffer_size);
}

size_t parser_deserialize_configuration_items(parser_t* parser, char* buffer, size_t buffer_size, configuration_item_command_t *first_config_item, size_t num_config_items)
{
    return parser->deserialize_configuration_items(buffer, buffer_size, first_config_item, num_config_items);
}
