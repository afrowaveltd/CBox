#ifndef CBOX_BLACKBOARD_OUTPUT_H
#define CBOX_BLACKBOARD_OUTPUT_H

#include <cbox/blackboard/errors.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief Represents the result of a write operation.
     *
     * This structure contains information about the outcome of a write operation, including any error that occurred,
     * the number of bytes successfully written, and an optional message describing the result.
     * @param error The error code indicating the type of error that occurred, if any.
     * @param written The number of bytes successfully written before any error occurred.
     * @param message An optional message providing additional information about the result.
     *
     * @note The structure is intended to provide a comprehensive summary of the write operation's outcome.
     *
     */
    typedef struct
    {
        cbox_console_error error; /**< Operation error, or NONE. */
        size_t written;           /**< Number of bytes written. */
        char message[128];        /**< Optional null-terminated diagnostic. */
    } cbox_write_result;

    /**
     * @brief Writes a specified number of bytes to an output stream.
     *
     * @return A cbox_write_result structure containing the outcome of the write operation.
     * @param output Destination stream.
     * @param text Source buffer.
     * @param length Number of bytes to write.
     * @note The function does not interpret UTF-8 or any other character encoding; it treats the input as raw bytes
     * @note The caller retains ownership of the source buffer and is responsible for managing its memory.
     */

    cbox_write_result cbox_blackboard_write_literal(
        FILE *output,
        const char *text,
        size_t length);

#ifdef __cplusplus
}
#endif

#endif // CBOX_BLACKBOARD_OUTPUT_H