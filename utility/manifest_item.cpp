#include "manifest_item.h"
#include "size_definitions.h"
#include "wolk_utils.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

void manifest_item_init(manifest_item_t* item, char* reference, reading_type_t reading_type, data_type_t data_type)
{
    /* Sanity check*/
    WOLK_ASSERT(strlen(reference) <= MANIFEST_ITEM_REFERENCE_SIZE);

    strcpy(item->reference, reference);

    item->reading_type = reading_type;
    item->data_type = data_type;

    item->data_dimensions = 1;

    memset(item->data_delimiter, '\0', MANIFEST_ITEM_DATA_DELIMITER_SIZE);
}

void manifest_item_set_reading_dimensions_and_delimiter(manifest_item_t* item, size_t data_size, char* delimiter)
{
    /* Sanity check */
    WOLK_ASSERT(data_size > 1);
    WOLK_ASSERT(strlen(delimiter) <= MANIFEST_ITEM_DATA_DELIMITER_SIZE);

    item->data_dimensions = data_size;
    strcpy(item->data_delimiter, delimiter);
}

char* manifest_item_get_reference(manifest_item_t* item)
{
    return item->reference;
}

data_type_t manifest_item_get_data_type(manifest_item_t* item)
{
    return item->data_type;
}

reading_type_t manifest_item_get_reading_type(manifest_item_t* item)
{
    return item->reading_type;
}

size_t manifest_item_get_data_dimensions(manifest_item_t* item)
{
    return item->data_dimensions;
}

char* manifest_item_get_data_delimiter(manifest_item_t* item)
{
    return item->data_delimiter;
}
