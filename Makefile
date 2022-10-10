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
CSTD  ?=-std=c++17
RELEASE_OPTIM?= -O3 -flto

PROJECT_NAME:=ATmega32u4_Emulator

SRC_DIR         ?=src/
BUILD_DIR       ?=build/make/$(PLATFORM)_$(BUILD_MODE)/
OBJ_DIR         ?=$(BUILD_DIR)objs/$(PROJECT_NAME)/
DEPENDENCIES_DIR?=dependencies/

OUT_NAME?=lib$(PROJECT_NAME).a
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

MAKE_CMD:=make

CDEPFLAGS=-MMD -MF ${@:.o=.d}

OUT_PATH:=$(OUT_DIR)$(OUT_NAME)

SRC_FILES:=$(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES:=$(addprefix $(OBJ_DIR),${SRC_FILES:.cpp=.o})
DEP_FILES:=$(patsubst %.o,%.d,$(OBJ_FILES))

DEPENDENCIES_INCLUDE_PATHS:=dependencies/CPP_Utils/src
DEPENDENCIES_LIBS_DIR:=$(BUILD_DIR)dependencies/libs

DEP_LIBS_DIRS:=$(addprefix $(DEPENDENCIES_DIR),)

DEP_LIBS_DEPS:=dependencies/Makefile $(shell find $(ROOT_DIR)dependencies/ -name '*h' -o -name '*.c' -o -name '*.cpp')

DEP_INCLUDE_FLAGS:=$(addprefix -I,$(DEPENDENCIES_INCLUDE_PATHS))
DEP_LIBS_BUILD_DIR:=$(BUILD_DIR)

# rules:

.PHONY:all clean

all: $(OUT_PATH)

$(OUT_PATH): $(DEP_LIBS_BUILD_DIR)$(PROJECT_NAME)_depFile.dep $(OBJ_FILES)
	mkdir -p $(OUT_DIR)
	$(AR) rvs $@ $(OBJ_FILES)

$(OBJ_DIR)%.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) $(CSTD) $(BUILD_MODE_CFLAGS) $(DEP_INCLUDE_FLAGS) -c $< -o $@ $(CDEPFLAGS)

-include $(DEP_FILES)

$(DEP_LIBS_BUILD_DIR)$(PROJECT_NAME)_depFile.dep: $(DEP_LIBS_DEPS)
	$(MAKE_CMD) -C $(DEPENDENCIES_DIR) PLATFORM=$(PLATFORM) BUILD_MODE=$(BUILD_MODE) CFLAGS="$(CFLAGS)" CSTD="$(CSTD)" RELEASE_OPTIM="$(RELEASE_OPTIM)" BUILD_DIR="$(DEP_LIBS_BUILD_DIR)"

clean:
	$(MAKE_CMD) -C $(DEPENDENCIES_DIR) clean BUILD_DIR=$(DEP_LIBS_BUILD_DIR)
	rm -rf $(BUILD_DIR)