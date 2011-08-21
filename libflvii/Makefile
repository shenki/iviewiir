# Makefile for libflvii
# Andrew Jeffery <andrew@aj.id.au>

SONAME := libflvii.so
VERSION := v0.1

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

DEPS := librtmp
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --cflags $(DEPS)) \
	-fPIC $(EXTRA_CFLAGS) $(WARNINGS)
LDFLAGS += -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME) $(EXTRA_LDFLAGS)
LDLIBS := $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --libs $(DEPS))

SRC := destroy_ctx.c \
    	destroy_tag_body.c \
	extract_tag.c \
    	extract_tag_body.c \
    	get_last_keyframe.c \
    	get_first_tag.c \
    	get_last_tag.c \
    	get_next_tag.c \
    	get_prev_tag.c \
    	is_flv.c \
    	is_last_tag.c \
    	new_ctx.c \
	get_duration.c

OBJ := $(SRC:.c=.o)
-include $(OBJ:.o=.d)

TEST_PROGRAM := ./test/test-executor
TEST_SRC := test/CuTest.c \
    	$(TEST_PROGRAM).c
TEST_OBJ := $(TEST_SRC:.c=.o)

check: CFLAGS += -DLIBFLVII_TEST -ggdb

all: libflvii.so libflvii.a

libflvii.so: $(OBJ)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $> $(LDLIBS)

static: libflvii.a

libflvii.a: $(OBJ)
	$(AR) rs $@ $?

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJ) $(OBJ:.o=.d) libflvii.so libflvii.a *.pc

libflvii.pc: libflvii.pc.in
	sed -e "s;@prefix@;$(PREFIX);" -e "s;@VERSION@;$(VERSION);" \
		libflvii.pc.in > $@

install: libflvii.a libflvii.pc
	-mkdir -p $(INCDIR) $(LIBDIR)/pkgconfig
	cp flvii.h $(INCDIR)
	cp libflvii.a $(LIBDIR)
	cp libflvii.pc $(LIBDIR)/pkgconfig

check: $(TEST_OBJ) libflvii.a
	$(CC) $^ $> -o $(TEST_PROGRAM) $(LDLIBS)
	$(TEST_PROGRAM)

.PHONY: clean
