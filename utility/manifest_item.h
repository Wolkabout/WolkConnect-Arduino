#ifndef SENSOR_H
#define SENSOR_H

#include "size_definitions.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    READING_TYPE_SENSOR = 0,
    READING_TYPE_ACTUATOR
} reading_type_t;

typedef enum {
    DATA_TYPE_NUMERIC = 0,
    DATA_TYPE_BOOLEAN,
    DATA_TYPE_STRING
} data_type_t;

typedef struct {
    char reference[MANIFEST_ITEM_REFERENCE_SIZE];

    reading_type_t reading_type;
    data_type_t data_type;

    size_t data_dimensions;

    char data_delimiter[MANIFEST_ITEM_DATA_DELIMITER_SIZE];
} manifest_item_t;

void manifest_item_init(manifest_item_t* item, char* reference, reading_type_t reading_type, data_type_t data_type);
void manifest_item_set_reading_dimensions_and_delimiter(manifest_item_t* item, size_t data_size, char* delimiter);

char* manifest_item_get_reference(manifest_item_t* item);

data_type_t manifest_item_get_data_type(manifest_item_t* item);

reading_type_t manifest_item_get_reading_type(manifest_item_t* item);

size_t manifest_item_get_data_dimensions(manifest_item_t* item);
char* manifest_item_get_data_delimiter(manifest_item_t* item);

#ifdef __cplusplus
}
#endif

#endif
