#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_REFERENCE_SIZE = 64,
    /* Maximum number of characters in reference string */
    MANIFEST_ITEM_DATA_DELIMITER_SIZE= 5,

    /* Maximum number of characters in reading value string */
    READING_SIZE = 96,
    /* Maximum number of reading dimensions (Data size on DV-Tool) */
    READING_DIMENSIONS = 3,

    /* Maximum number of characters in actuation value string */
    COMMAND_ARGUMENT_SIZE = READING_SIZE,

    /* Maximum number of characters in configuration item name string */
    CONFIGURATION_ITEM_NAME_SIZE = MANIFEST_ITEM_REFERENCE_SIZE,
    /* Maximum number of characters in configuration item value string */
    CONFIGURATION_ITEM_VALUE_SIZE = READING_SIZE,

    /* Parser internal buffer size, should be at least READING_SIZE  big */
    PARSER_INTERNAL_BUFFER_SIZE = READING_SIZE
};

#ifdef __cplusplus
}
#endif

#endif
