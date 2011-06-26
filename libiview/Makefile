# Makefile for libiview
# Joel Stanley <joel@jms.id.au>

SONAME := libiview.so
VERSION := v0.5

# Uncomment when building for the wii, or provide on the command line
# CROSS_COMPILE=powerpc-eabi-

ifeq ($(CROSS_COMPILE),powerpc-eabi-)
PKGCONFIG := /opt/devkitpro/portlibs/ppc/lib/pkgconfig
EXTRA_CFLAGS := -O3 -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DHW_RVL \
		-I/opt/devkitpro/libogc/include	\
		-I/opt/devkitpro/libogc/lib/wii
EXTRA_LDFLAGS = -L/opt/devkitpro/devkitPPC/powerpc-eabi/lib \
		-L/opt/devkitpro/libogc/lib/wii
PREFIX := /opt/devkitpro/portlibs/ppc

# Hack as devkitpro does not have a -cc symlink
CC:=gcc
else
PREFIX := /usr/include
endif

CC	:= $(CROSS_COMPILE)$(CC)
AR	:= $(CROSS_COMPILE)$(AR)

INCDIR := $(PREFIX)/include
LIBDIR := $(PREFIX)/lib

WARNINGS := -Wall -Wextra -Wwrite-strings -Werror

DEPS := libxml-2.0 json librtmp
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --cflags $(DEPS)) \
	-fPIC $(EXTRA_CFLAGS) $(WARNINGS)
LDFLAGS += -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME) $(EXTRA_LDFLAGS)
LDLIBS := $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --libs $(DEPS))

SRC := get_http_buffer.c \
	parse_config.c \
	get_index.c \
	parse_index.c \
	get_series.c \
	parse_series.c \
	destroy_config.c \
	destroy_index.c \
	get_auth.c \
	fetch_episode.c \
	destroy_series.c \
	destroy_auth.c \
	wiicode/http.c \
	strtrim.c \
	find_series.c \
	find_episode.c \
	easy_config.c \
	easy_index.c \
	easy_series.c \
	easy_fetch_episode.c \
	strrpl.c \
	parse_auth.c \
	easy_auth.c \
	destroy_http_buffer.c \
	get_categories.c \
	destroy_categories.c

OBJ := $(SRC:.c=.o)

TEST_PROGRAM := ./test/test-executor
TEST_SRC := test/CuTest.c \
    	$(TEST_PROGRAM).c
TEST_OBJ := $(TEST_SRC:.c=.o)

check: CFLAGS += -DLIBIVIEW_TEST -ggdb

all: libiview.so libiview.a

libiview.so: $(OBJ)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $> $(LDLIBS)

static: libiview.a

libiview.a: $(OBJ)
	$(AR) rs $@ $?

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJ) $(OBJ:.o=.d) libiview.so libiview.a *.pc

libiview.pc: libiview.pc.in
	sed -e "s;@prefix@;$(PREFIX);" -e "s;@VERSION@;$(VERSION);" \
		libiview.pc.in > $@

install: libiview.a libiview.pc
	-mkdir -p $(INCDIR) $(LIBDIR)/pkgconfig
	cp iview.h $(INCDIR)
	cp libiview.a $(LIBDIR)
	cp libiview.pc $(LIBDIR)/pkgconfig

check: $(TEST_OBJ) libiview.a
	$(CC) $^ $> -o $(TEST_PROGRAM) $(LDLIBS)
	$(TEST_PROGRAM)

.PHONY: clean
