#ifndef JSON_SERIALIZER_H
#define JSON_SERIALIZER_H

#include "actuator_command.h"
#include "configuration_item.h"
#include "configuration_item_command.h"
#include "reading.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

size_t json_serialize_readings(reading_t* first_reading, size_t num_readings, char* buffer, size_t buffer_size);

size_t json_deserialize_commands(char* buffer, size_t buffer_size, actuator_command_t* commands_buffer, size_t commands_buffer_size);

size_t json_serialize_configuration_items(configuration_item_t* first_config_item, size_t num_config_items, char* buffer, size_t buffer_size);

size_t json_deserialize_configuration_items(char* buffer, size_t buffer_size, configuration_item_command_t* commands_buffer, size_t commands_buffer_size);

#ifdef __cplusplus
}
#endif

#endif
