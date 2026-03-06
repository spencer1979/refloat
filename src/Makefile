# WARNING: If you use this Makefile to build, you need to generate config files
# from settings.xml using this file as well, otherwise Refloat Cfg will not
# load in vesc_tool, see comment below.

VESC_TOOL ?= ../vesc_tool_6.06.exe
BUILD_DIR = ../build

TARGET = package_lib
BUILD_DIR = ../build

all: copy_build

REFLOAT_SOURCES = $(wildcard *.c) $(wildcard lib/*.c)
CONF_GEN_HEADERS = conf/conf_default.h conf/confparser.h conf/confxml.h
CONF_GEN_SOURCES = conf/confparser.c conf/confxml.c
SOURCES = $(REFLOAT_SOURCES) $(CONF_GEN_SOURCES) conf/buffer.c
DEPS = $(SOURCES:.c=.d)

VESC_C_LIB_PATH = ../vesc_pkg_lib/
include $(VESC_C_LIB_PATH)rules.mk

CFLAGS += -MMD -flto -ggdb
LDFLAGS += -flto

$(REFLOAT_SOURCES): $(CONF_GEN_HEADERS) conf/conf_general.h

# Generate config files from settings.xml
$(CONF_GEN_HEADERS) $(CONF_GEN_SOURCES) &: conf/settings.xml
	$(VESC_TOOL) --xmlConfToCode conf/settings.xml
	# !!! make xml config loading in vesc_tool work with LTO !!!
	sed -i "s/^uint8_t data_/__attribute__((used)) uint8_t data_/g" conf/confxml.c

PACKAGE_NAME=`cat ../package_name | cut -c-20`
VERSION=`cat ../version`
MAJOR=`cat ../version | cut -d. -f1`
MINOR=`cat ../version | cut -d. -f2`
PATCH=`cat ../version | cut -d. -f3 | cut -d- -f1`
SUFFIX=`cat ../version | cut -d. -f3 | cut -d- -f2- | cut -c-20`
GIT_HASH=`git rev-parse --short=8 HEAD | cut -c-8`

conf/conf_general.h: conf/conf_general.h.in ../package_name ../version
	cat $< | \
	sed "s/{{PACKAGE_NAME}}/${PACKAGE_NAME}/g" | \
	sed "s/{{VERSION}}/${VERSION}/g" | \
	sed "s/{{MAJOR_VERSION}}/${MAJOR}/g" | \
	sed "s/{{MINOR_VERSION}}/${MINOR}/g" | \
	sed "s/{{PATCH_VERSION}}/${PATCH}/g" | \
	sed "s/{{VERSION_SUFFIX}}/${SUFFIX}/g" | \
	sed "s/{{GIT_HASH}}/${GIT_HASH}/g" > $@

# Copy output to build directory
.PHONY: copy_build
copy_build: $(TARGET)
	mkdir -p $(BUILD_DIR)
	cp $(TARGET).so $(TARGET).bin $(TARGET).lisp $(BUILD_DIR)/ 2>/dev/null || true

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) *.so *.d *.o *.elf *.list *.lisp *.bin conf/conf_general.h conf/conf_default.h conf/confparser.* conf/confxml.* conf/utils/
