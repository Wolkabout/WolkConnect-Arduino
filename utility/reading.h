#ifndef READING_H
#define READING_H

#include "size_definitions.h"
#include "manifest_item.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ACTUATOR_STATUS_READY = 0,
    ACTUATOR_STATUS_BUSY,
    ACTUATOR_STATUS_ERROR
} actuator_status_t;

typedef struct {
    manifest_item_t manifest_item;

    actuator_status_t actuator_status;

    char reading_data[READING_DIMENSIONS][READING_SIZE];

    uint32_t rtc;
} reading_t;

void reading_init(reading_t* reading, manifest_item_t* item);

void reading_clear(reading_t* reading);
void reading_clear_array(reading_t* first_reading, size_t readings_count);

void reading_set_data(reading_t* reading, char* data);
char* reading_get_data(reading_t* reading);

bool reading_get_delimited_data(reading_t* reading, char* buffer, size_t buffer_size);

void reading_set_data_at(reading_t* reading, char* data, size_t data_position);
char* reading_get_data_at(reading_t* reading, size_t data_position);

manifest_item_t* reading_get_manifest_item(reading_t* reading);

void reading_set_rtc(reading_t* reading, uint32_t rtc);
uint32_t reading_get_rtc(reading_t* reading);

void reading_set_actuator_status(reading_t* reading, actuator_status_t actuator_status);
actuator_status_t reading_get_actuator_status(reading_t* reading);

#ifdef __cplusplus
}
#endif

#endif
