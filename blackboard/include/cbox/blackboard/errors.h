#ifndef CBOX_BLACKBOARD_ERRORS_H
#define CBOX_BLACKBOARD_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Represents different types of console errors.
     */
    typedef enum
    {
        CBOX_CONSOLE_ERROR_NONE = 0,         /**< No error. */
        CBOX_CONSOLE_ERROR_INVALID_ARGUMENT, /**< Invalid argument. */
        CBOX_CONSOLE_ERROR_INPUT_DATA,       /**< Invalid input data. */
        CBOX_CONSOLE_ERROR_ACCESS_DENIED,    /**< Access denied. */
        CBOX_CONSOLE_ERROR_IO,               /**< I/O failure. */
        CBOX_CONSOLE_ERROR_TERMINAL,         /**< Terminal failure. */
        CBOX_CONSOLE_ERROR_UNSUPPORTED       /**< Unsupported operation. */
    } cbox_console_error;

#ifdef __cplusplus
}
#endif

#endif // CBOX_BLACKBOARD_ERRORS_H
