# CBox: initial C17 toolchain smoke test.
# No library is compiled yet. Module targets will be added after API design.

CC = cc
CFLAGS = -std=c17 -O0 -g -Wall -Wextra -Wpedantic
CPPFLAGS =
LDFLAGS =
LDLIBS =
BUILD_DIR = build

.PHONY: all help test clean
.DEFAULT_GOAL := help

help:
	@printf "CBox foundation (no library implementations yet)\n"
	@printf "  make test   - compile and run C17 toolchain smoke test\n"
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
