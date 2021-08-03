# settings here:

BUILD_MODE?=DEBUG

CC    :=g++
CFLAGS?=-Wall -Wno-narrowing
CSTD  ?=-std=c++17
RELEASE_OPTIM?= -O2

SRC_DIR         ?=src/
BUILD_DIR       ?=build/make/
OBJ_DIR         ?=$(BUILD_DIR)objs/ATmega32u4_Emulator/

OUT_NAME?=ATmega32u4_Emulator.a
OUT_DIR ?=$(BUILD_DIR)

# you dont need to worry about this stuff:

# detect OS
ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

# get current dir
current_dir :=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_MODE_CFLAGS:=
ifeq ($(BUILD_MODE),DEBUG)
	BUILD_MODE_CFLAGS += -g
else
	BUILD_MODE_CFLAGS +=$(RELEASE_OPTIM)
endif

ifeq ($(detected_OS),Windows)
	BASH_PREFX:=bash -c 
endif

CDEPFLAGS=-MMD -MF ${@:.o=.d}

OUT_PATH:=$(OUT_DIR)$(OUT_NAME)

SRC_FILES:=$(shell $(BASH_PREFX)"find $(SRC_DIR) -name '*.cpp'")
OBJ_FILES:=$(addprefix $(OBJ_DIR),${SRC_FILES:.cpp=.o})
DEP_FILES:=$(patsubst %.o,%.d,$(OBJ_FILES))

# rules:

.PHONY:all clean

all: $(OUT_PATH)

$(OUT_PATH): $(OBJ_FILES)
	# BUILDING ATmega32u4_Emulator
	$(BASH_PREFX)"mkdir -p $(OUT_DIR)"
	ar rvs $@ $(OBJ_FILES)

$(OBJ_DIR)%.o:%.cpp
	$(BASH_PREFX)"mkdir -p $(dir $@)"
	$(CC) $(CFLAGS) $(CSTD) $(BUILD_MODE_CFLAGS) $(DEP_INCLUDE_FLAGS) -c $< -o $@ $(CDEPFLAGS)

-include $(DEP_FILES)

clean:
	rm -rf $(BUILD_DIR)