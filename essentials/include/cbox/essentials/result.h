#ifndef CBOX_ESSENTIALS_RESULT_H
#define CBOX_ESSENTIALS_RESULT_H

/**
 * @file result.h
 * @brief Minimal, provisional C17 result contract for the CBox teaching lab.
 *
 * The API is a learning scaffold, not a stable released interface.
 * Result contains no data payload; Response will be designed separately.
 */

/**
 * @brief Distinguishes a successful operation from a failure.
 *
 * UNKNOWN is zero so a zero-initialized result is not silently successful.
 * The numeric values are CBox-local and are not a cross-language wire format.
 */
typedef enum
{
    CBOX_RESULT_UNKNOWN = 0,
    CBOX_RESULT_OK = 1,
    CBOX_RESULT_FAIL = 2
} cbox_result_status;

/**
 * @brief Outcome of an operation with no data payload.
 *
 * Invariant for results built by our constructors:
 * - OK: status=OK, error_code=0, message=NULL.
 * - FAIL: status=FAIL; error_code can be 0 when no code is available;
 *   message can be NULL when no description is available.
 *
 * @note The message pointer is BORROWED. CBox does not allocate, copy,
 *       or release it. If non-NULL, the referenced characters must remain
 *       valid for as long as a caller uses the message.
 * @note Copying this struct copies the pointer value, not pointed-to text.
 */
typedef struct
{
    cbox_result_status status;
    int error_code;
    const char *message;
} cbox_result;

/**
 * @brief Create a successful result without data.
 *
 * @return A value with status OK, error_code 0, and message NULL.
 * @note No dynamic allocation; return the struct by value.
 * @note Safe to use and copy without a matching cleanup function.
 */
cbox_result cbox_result_ok(void);

/**
 * @brief Create a failed result without data.
 *
 * @param error_code Application-defined numeric code; 0 means unspecified.
 * @param message Optional BORROWED NUL-terminated error description,
 *                or NULL. May point to a string literal.
 * @return A value with status FAIL and the supplied error details.
 * @note No dynamic allocation or copying; the caller retains ownership
 *       of message and must keep it alive while the result is used.
 * @warning Do not pass a pointer to a function-local character array and
 *          then use the returned message after that array's lifetime ends.
 */
cbox_result cbox_result_fail(int error_code, const char *message);

#endif
