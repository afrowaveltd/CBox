# CBox: initial C17 toolchain smoke test.
# No library is compiled yet. Module targets will be added after API design.

CC = cc
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -Wpedantic
CPPFLAGS = -Iessentials/include
LDFLAGS =
LDLIBS =
BUILD_DIR = build

.PHONY: all help test test-result clean
.DEFAULT_GOAL := help

help:
	@printf "CBox foundation (no library implementations yet)\n"
	@printf "  make test   - compile and run C17 toolchain smoke test\n"
	@printf "  make test-result - Result unit tests (RED until you write result.c)\n"
	@printf "  make all    - build the smoke test program\n"
	@printf "  make clean  - remove generated build files\n"
	@printf "  make help   - show this help\n"

all: $(BUILD_DIR)/smoke_test

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/smoke_test: tests/smoke_test.c | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

test: $(BUILD_DIR)/smoke_test
	./$(BUILD_DIR)/smoke_test

clean:
	rm -rf $(BUILD_DIR)

# Guided Result exercise. This target is intentionally RED until the
# learner writes essentials/src/result.c. The normal smoke test remains green.
$(BUILD_DIR)/test_result: essentials/tests/test_result.c essentials/src/result.c essentials/include/cbox/essentials/result.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) essentials/tests/test_result.c essentials/src/result.c $(LDFLAGS) $(LDLIBS) -o $@

test-result: $(BUILD_DIR)/test_result
	./$(BUILD_DIR)/test_result
