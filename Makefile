# ==================================================
# Makefile for LINC
# ==================================================

# ==================================================
# Variables
# ==================================================

SRC_DIR ?= src
INC_DIR ?= include
TEST_DIR ?= test
BUILD_DIR ?= build
OBJ_DIR := $(BUILD_DIR)/objects
BIN_DIR := $(BUILD_DIR)/binaries

BEAR_FILE ?= $(BUILD_DIR)/compile_commands.json

# MAIN_SOURCE := $(SRC_DIR)/main.c
# MAIN_OBJECT := $(OBJ_DIR)/$(MAIN_SOURCE:.c=.o)
# MAIN_TARGET := $(BIN_DIR)/$(SRC_DIR)/bin
# MAIN_DEPS := $(OBJ_DIR)/$(MAIN_SOURCE:.c=.d)

SRC_SOURCES := $(shell find $(SRC_DIR) -name '*.c' -not -wholename '$(MAIN_SOURCE)')
SRC_OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC_SOURCES))
SRC_DEPS := $(patsubst %.c,$(OBJ_DIR)/%.d,$(SRC_SOURCES))

TEST_SOURCES := $(shell find $(TEST_DIR) -name '*.c')
TEST_OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(TEST_SOURCES))
TEST_TARGETS := $(patsubst %.c,$(BIN_DIR)/%,$(TEST_SOURCES))
TEST_DEPS := $(patsubst %.c,$(OBJ_DIR)/%.d,$(TEST_SOURCES))

# ==================================================
# Compiler and flags
# ==================================================

# Compiler
CC := gcc

# Bear integration
BEAR ?= 1
ifeq ($(BEAR),1)
CCWRAP := bear --output $(BEAR_FILE) --append -- $(CC)
else
CCWRAP := $(CC)
endif

# Preprocessor flags
CPPFLAGS ?= -D_POSIX_C_SOURCE=199309L -I$(INC_DIR)

# Compiler flags
CFLAGS ?= -Wall -Wextra -Werror -std=c99 -pedantic-errors

# Linker flags
LDFLAGS ?= -lpthread -lm

# Dependency flags
DFLAGS ?= -MMD -MP

# ==================================================
# Files rules
# ==================================================

# Makefile variable to set the default rule for `make`
.DEFAULT_GOAL := all

# Rules for building object files. Compiling each source file into an object file.
$(OBJ_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CCWRAP) $(CPPFLAGS) $(CFLAGS) $(DFLAGS) -c $< -o $@

# Rules for building the final binaries. Linking object files into executables.
# $(MAIN_TARGET): $(MAIN_OBJECT) $(SRC_OBJECTS)
# 	mkdir -p $(@D)
# 	$(CCWRAP) $^ $(LDFLAGS) -o $@

$(BIN_DIR)/$(TEST_DIR)/%: $(OBJ_DIR)/$(TEST_DIR)/%.o $(SRC_OBJECTS)
	mkdir -p $(@D)
	$(CCWRAP) $^ $(LDFLAGS) -o $@

# ==================================================
# Phony rules
# ==================================================

.PHONY: all
all: compile tests

.PHONY: compile
compile: $(MAIN_OBJECT) $(SRC_OBJECTS)

# .PHONY: run
# run: $(MAIN_TARGET)
# 	./$<

.PHONY: tests
tests: $(TEST_TARGETS)

.PHONY: run-tests
run-tests: $(TEST_TARGETS)
	for test in $^; do ./$$test || exit 1; done

# Rule to print the value of a variable.
.PHONY: vars-%
vars-%:
	@echo "$* = $($*)"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BEAR_FILE)

# ==================================================
# Environment rules
# ==================================================

.PHONY: packages
packages:
	sudo apt install bear

.PHONY: unpackages
unpackages:
	sudo apt remove bear

# Include dependency files for automatic dependency tracking.
-include $(SRC_DEPS)
-include $(MAIN_DEPS)
-include $(TEST_DEPS)
