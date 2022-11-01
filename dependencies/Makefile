# settings here:

BUILD_MODE?=DEBUG
PLATFORM?=PLATFORM_DESKTOP

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CC:=gcc
    CXX:=g++
	AR:=ar
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC:=emcc
    CXX:=em++
	AR:=emar
endif
CFLAGS?=-Wall -Wno-narrowing $(CUSTOM_CFLAGS)
CSTD:=-std=c++17
RELEASE_OPTIM?= -O2

BUILD_DIR?=build/make/$(PLATFORM)_$(BUILD_MODE)/

# you dont need to worry about this stuff:

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

MAKE_CMD?=make

ifeq ($(BUILD_DIR),build/)
	BUILD_DIR:=$(current_dir)$(BUILD_DIR)
endif

CDEPFLAGS=-MMD -MF ${@:.o=.d}


CPP_UTILS_DIR:=CPP_Utils/

CPP_UTILS_BUILD_DIR:=$(BUILD_DIR)
CPP_UTILS_SRC_DIR:=$(CPP_UTILS_DIR)src/
CPP_UTILS_DEP_DIR:=$(CPP_UTILS_DIR)dependencies/

CPP_UTILS_OUTNAME:=libCPP_Utils.a
CPP_UTILS_OUTPATH:=$(CPP_UTILS_BUILD_DIR)$(CPP_UTILS_OUTNAME)
CPP_UTILS_DEPS:=$(shell find $(CPP_UTILS_SRC_DIR) -name '*.*') $(shell find $(CPP_UTILS_DEP_DIR) -name '*.*')


# rules:

.PHONY:all clean

all: $(CPP_UTILS_OUTPATH)
	mkdir -p $(BUILD_DIR)
	touch $(BUILD_DIR)ATmega32u4_Emulator_depFile.dep

$(CPP_UTILS_OUTPATH):$(CPP_UTILS_DEPS)
	mkdir -p $(CPP_UTILS_BUILD_DIR)
	$(MAKE_CMD) -C "$(CPP_UTILS_DIR)" PLATFORM=$(PLATFORM) BUILD_MODE=$(BUILD_MODE) BUILD_DIR="$(CPP_UTILS_BUILD_DIR)" RELEASE_OPTIM=$(RELEASE_OPTIM) "CUSTOM_CFLAGS=$(CUSTOM_CFLAGS)"

clean:
	$(MAKE_CMD) -C "$(RAYLIB_DIR)/src" clean
	rm -rf $(BUILD_DIR)