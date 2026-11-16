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
     * @brief Writes a specified number of literal bytes.
     *
     * @param output Destination stream; must not be NULL.
     * @param text Source buffer; must contain at least length readable bytes
     *             when length is greater than zero.
     * @param length Number of bytes to write.
     *
     * @return A cbox_write_result containing the error code,
     *         number of bytes written, and optional diagnostic.
     *
     * @note The input is treated as raw bytes.
     * @note The function does not interpret UTF-8 or markup.
     * @note The caller retains ownership of the source buffer.
     */

    cbox_write_result cbox_blackboard_write_literal(
        FILE *output,
        const char *text,
        size_t length);

#ifdef __cplusplus
}
#endif

#endif // CBOX_BLACKBOARD_OUTPUT_H