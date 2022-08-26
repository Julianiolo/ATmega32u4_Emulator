# settings here:

BUILD_MODE?=DEBUG
PLATFORM?=PLATFORM_DESKTOP

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CC?=gcc
    CXX?=g++
	AR:=ar
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC:=emcc
    CXX:=em++
	AR:=emar
endif
CFLAGS?=-Wall -Wno-narrowing
CSTD  ?=-std=gnu++17
RELEASE_OPTIM?= -O3 -flto

SRC_DIR         ?=src/
BUILD_DIR       ?=build/make/$(PLATFORM)_$(BUILD_MODE)/
OBJ_DIR         ?=$(BUILD_DIR)objs/ATmega32u4_Emulator/

OUT_NAME?=libATmega32u4_Emulator.a
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

CDEPFLAGS=-MMD -MF ${@:.o=.d}

OUT_PATH:=$(OUT_DIR)$(OUT_NAME)

SRC_FILES:=$(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES:=$(addprefix $(OBJ_DIR),${SRC_FILES:.cpp=.o})
DEP_FILES:=$(patsubst %.o,%.d,$(OBJ_FILES))

# rules:

.PHONY:all clean

all: $(OUT_PATH)

$(OUT_PATH): $(OBJ_FILES)
	# BUILDING ATmega32u4_Emulator
	mkdir -p $(OUT_DIR)
	$(AR) rvs $@ $(OBJ_FILES)

$(OBJ_DIR)%.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CSTD) $(BUILD_MODE_CFLAGS) $(DEP_INCLUDE_FLAGS) -c $< -o $@ $(CDEPFLAGS)

-include $(DEP_FILES)

clean:
	rm -rf $(BUILD_DIR)