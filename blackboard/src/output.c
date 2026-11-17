#include <cbox/blackboard/output.h>
#include <stdio.h>
#include <string.h>

cbox_write_result cbox_blackboard_write_literal(
    FILE *output,
    const char *text,
    size_t length)
{
    cbox_write_result result = {0};
    if (output == NULL || (text == NULL && length > 0))
    {
        result.error = CBOX_CONSOLE_ERROR_INVALID_ARGUMENT;
        return result;
    }
    if (length == 0)
    {
        return result;
    }

    size_t written = fwrite(text, 1, length, output);

    result.written = written;
    if (result.written != length)
    {
        result.error = CBOX_CONSOLE_ERROR_IO;
        return result;
    }
    return result;
}
