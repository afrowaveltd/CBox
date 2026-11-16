#ifndef CBOX_BLACKBOARD_OUTPUT_H
#define CBOX_BLACKBOARD_OUTPUT_H

#include <stddef.h>
#include <stdio.h>

/**
 * @brief Writes a specified number of bytes to an output stream.
 * 
 * @param output Destination stream.
 * @param text Source buffer.
 * @param length Number of bytes to write.
 * 
 * @return Number of bytes written, or a negative value if an error occurs.
 * 
 * @note The function does not interpret UTF-8 or any other character encoding; it treats the input as raw bytes
 * @note The caller retains ownership of the source buffer and is responsible for managing its memory.
 */

size_t cbox_blackboard_write_literal(
    FILE *output,
    const char *text,
    size_t length
);

#endif // CBOX_BLACKBOARD_OUTPUT_H