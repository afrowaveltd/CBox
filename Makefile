# CBox: portable C17 tools plus optional external DoesItFail integration.
# Production CBox modules must not depend on DoesItFail.

CC = cc
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -Wpedantic
CPPFLAGS = -Iessentials/include
LDFLAGS =
LDLIBS =
BUILD_DIR = build
DOES_IT_FAIL_DIR ?= ../DoesItFail

.PHONY: all help test test-result test-framework clean
.DEFAULT_GOAL := help

help:
	@printf "CBox foundation and guided Result exercise\n"
	@printf "  make test           - compile/run standalone C17 smoke test\n"
	@printf "  make test-result    - Result unit tests (requires your local result.c)\n"
	@printf "  make test-framework - optional separate DoesItFail library integration\n"
	@printf "  make all            - build smoke test program\n"
	@printf "  make clean          - remove generated build files\n"
	@printf "  make help           - show this help\n"

all: $(BUILD_DIR)/smoke_test

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/smoke_test: tests/smoke_test.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

test: $(BUILD_DIR)/smoke_test
	./$(BUILD_DIR)/smoke_test

clean:
	rm -rf $(BUILD_DIR)

# Result teaching exercise: untouched; the learner writes result.c.
$(BUILD_DIR)/test_result: essentials/tests/test_result.c essentials/src/result.c essentials/include/cbox/essentials/result.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) essentials/tests/test_result.c essentials/src/result.c $(LDFLAGS) $(LDLIBS) -o $@

test-result: $(BUILD_DIR)/test_result
	./$(BUILD_DIR)/test_result

# Optional: independent library is only needed for this test target.
test-framework: | $(BUILD_DIR)
	@test -f "$(DOES_IT_FAIL_DIR)/Makefile" || { \
		printf 'DoesItFail not found at %s\nSee Docs/DOES_IT_FAIL_INTEGRATION.md\n' "$(DOES_IT_FAIL_DIR)" >&2; \
		exit 2; \
	}
	$(MAKE) -C "$(DOES_IT_FAIL_DIR)" all
	$(CC) $(CPPFLAGS) $(CFLAGS) -I"$(DOES_IT_FAIL_DIR)/include" \
		tests/does_it_fail_smoke.c "$(DOES_IT_FAIL_DIR)/build/libdoesitfail.a" \
		$(LDFLAGS) $(LDLIBS) -o "$(BUILD_DIR)/test_framework"
	./$(BUILD_DIR)/test_framework
