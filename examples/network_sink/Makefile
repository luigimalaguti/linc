SRC_DIR ?= src
INC_DIR ?= include
BUILD_DIR ?= build
OBJ_DIR := $(BUILD_DIR)/objects
BIN_DIR := $(BUILD_DIR)/binaries

EXT_DIR ?= ../..
LINC_DIR ?= $(EXT_DIR)

BEAR_FILE ?= ../../$(BUILD_DIR)/compile_commands.json

MAIN_SOURCE := $(SRC_DIR)/main.c
MAIN_OBJECT := $(OBJ_DIR)/$(MAIN_SOURCE:.c=.o)
MAIN_TARGET := $(BIN_DIR)/$(SRC_DIR)/bin
MAIN_DEPS := $(OBJ_DIR)/$(MAIN_SOURCE:.c=.d)

LOCAL_SOURCES := $(shell find $(SRC_DIR) -name '*.c' -not -wholename '$(MAIN_SOURCE)')
EXTERNAL_SOURCES := $(shell find $(LINC_DIR)/$(SRC_DIR) -name '*.c')
LOCAL_OBJECTS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(LOCAL_SOURCES))
EXTERNAL_OBJECTS := $(patsubst $(LINC_DIR)/%.c,$(OBJ_DIR)/external/%.o,$(EXTERNAL_SOURCES))
SRC_OBJECTS := $(LOCAL_OBJECTS) $(EXTERNAL_OBJECTS)
SRC_DEPS := $(patsubst %.o,%.d,$(SRC_OBJECTS))

CC := gcc
BEAR ?= 1
ifeq ($(BEAR),1)
CCWRAP := bear --output $(BEAR_FILE) --append -- $(CC)
else
CCWRAP := $(CC)
endif
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L -I$(INC_DIR) -I$(LINC_DIR)/$(INC_DIR)
CFLAGS ?= -Wall -Wextra -Werror -std=c99 -pedantic-errors
LDFLAGS ?= -lpthread -lm
DFLAGS ?= -MMD -MP

.DEFAULT_GOAL := all

$(OBJ_DIR)/%.o: %.c
	mkdir -p $(@D)
	$(CCWRAP) $(CPPFLAGS) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(OBJ_DIR)/external/%.o: $(LINC_DIR)/%.c
	mkdir -p $(@D)
	$(CCWRAP) $(CPPFLAGS) $(CFLAGS) $(DFLAGS) -c $< -o $@

$(MAIN_TARGET): $(MAIN_OBJECT) $(SRC_OBJECTS)
	mkdir -p $(@D)
	$(CCWRAP) $^ $(LDFLAGS) -o $@

.PHONY: all
all: compile

.PHONY: compile
compile: $(MAIN_OBJECT) $(SRC_OBJECTS)

.PHONY: run
run: $(MAIN_TARGET)
	./$<

.PHONY: wrk
wrk:
	wrk -t12 -c400 -d30s http://127.0.0.1:3000

.PHONY: vars-%
vars-%:
	@echo "$* = $($*)"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BEAR_FILE)
