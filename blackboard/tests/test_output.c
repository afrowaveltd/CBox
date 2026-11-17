#include "cbox/blackboard/output.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_write_literal(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    FILE *file = tmpfile();

    assert(file != NULL);
    *p_tests_passed += 1;
    const char text[] = "Hello";

    cbox_write_result result =
        cbox_blackboard_write_literal(file, text, 5);
    assert(result.error == CBOX_CONSOLE_ERROR_NONE);
    *p_tests_passed += 1;
    assert(result.written == 5);
    *p_tests_passed += 1;
    assert(result.message[0] == '\0');
    *p_tests_passed += 1;

    rewind(file);

    char buffer[5] = {0};
    size_t read = fread(buffer, 1, sizeof(buffer), file);
    assert(read == 5);
    *p_tests_passed += 1;
    assert(memcmp(buffer, text, 5) == 0);
    *p_tests_passed += 1;

    fclose(file);
    *p_cases_passed += 1;
}

void test_write_literal_empty(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    FILE *file = tmpfile();
    assert(file != NULL);
    *p_tests_passed += 1;

    const char text[] = "";

    cbox_write_result result =
        cbox_blackboard_write_literal(file, text, 0);
    assert(result.error == CBOX_CONSOLE_ERROR_NONE);
    *p_tests_passed += 1;
    assert(result.written == 0);
    *p_tests_passed += 1;
    assert(result.message[0] == '\0');
    *p_tests_passed += 1;

    rewind(file);

    char buffer[1] = {0};
    size_t read = fread(buffer, 1, sizeof(buffer), file);
    assert(read == 0);
    *p_tests_passed += 1;

    fclose(file);
    *p_cases_passed += 1;
}

void test_write_null_text_zero_length(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    FILE *file = tmpfile();
    assert(file != NULL);
    *p_tests_passed += 1;
    cbox_write_result result =
        cbox_blackboard_write_literal(file, NULL, 0);
    assert(result.error == CBOX_CONSOLE_ERROR_NONE);
    *p_tests_passed += 1;
    assert(result.written == 0);
    *p_tests_passed += 1;
    assert(result.message[0] == '\0');
    *p_tests_passed += 1;
    fclose(file);
    *p_cases_passed += 1;
}

void test_write_literal_null(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    FILE *file = tmpfile();
    assert(file != NULL);
    *p_tests_passed += 1;

    cbox_write_result result =
        cbox_blackboard_write_literal(file, NULL, 5);

    assert(result.error == CBOX_CONSOLE_ERROR_INVALID_ARGUMENT);
    *p_tests_passed += 1;
    assert(result.written == 0);
    *p_tests_passed += 1;

    fclose(file);
    *p_cases_passed += 1;
}

void test_write_invalid_file(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    cbox_write_result result =
        cbox_blackboard_write_literal(NULL, "Hello", 5);

    assert(result.error == CBOX_CONSOLE_ERROR_INVALID_ARGUMENT);
    *p_tests_passed += 1;
    assert(result.written == 0);
    *p_tests_passed += 1;
    *p_cases_passed += 1;
}

void test_write_null_file_null_text(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    cbox_write_result result =
        cbox_blackboard_write_literal(NULL, NULL, 0);

    assert(result.error == CBOX_CONSOLE_ERROR_INVALID_ARGUMENT);
    *p_tests_passed += 1;
    assert(result.written == 0);
    *p_tests_passed += 1;
    *p_cases_passed += 1;
}

void test_write_binary_literal(unsigned int *p_tests_passed, unsigned int *p_cases_passed)
{
    FILE *file = tmpfile();
    assert(file != NULL);
    *p_tests_passed += 1;

    const unsigned char data[] = {0x41, 0x00, 0x42, 0x00, 0x43};
    cbox_write_result result =
        cbox_blackboard_write_literal(file, (const char *)data, sizeof(data));

    assert(result.error == CBOX_CONSOLE_ERROR_NONE);
    *p_tests_passed += 1;
    assert(result.written == sizeof(data));
    *p_tests_passed += 1;

    rewind(file);

    unsigned char read_data[sizeof(data)];
    size_t read = fread(read_data, 1, sizeof(read_data), file);
    assert(read == sizeof(data));
    *p_tests_passed += 1;
    for (size_t i = 0; i < sizeof(data); ++i)
    {
        assert(read_data[i] == data[i]);
        *p_tests_passed += 1;
    }

    fclose(file);
    *p_cases_passed += 1;
}

int main(void)
{
    unsigned int tests_passed = 0;
    unsigned int cases_passed = 0;
    test_write_literal(&tests_passed, &cases_passed);
    test_write_literal_empty(&tests_passed, &cases_passed);
    test_write_null_text_zero_length(&tests_passed, &cases_passed);
    test_write_literal_null(&tests_passed, &cases_passed);
    test_write_invalid_file(&tests_passed, &cases_passed);
    test_write_null_file_null_text(&tests_passed, &cases_passed);
    test_write_binary_literal(&tests_passed, &cases_passed);
    printf("================================\n");
    printf("Tests passed: %u\n", cases_passed);
    printf("Asserts passed: %u\n", tests_passed);
    printf("================================\n");
    return 0;
}
