SONAME := libiview.so
VERSION := v0.3


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
else
PREFIX := /usr/include
endif

CC	:=$(CROSS_COMPILE)gcc
AR	:=$(CROSS_COMPILE)ar

INCDIR := $(PREFIX)/include
LIBDIR := $(PREFIX)/lib

WARNINGS := -Wall -Wextra -Wwrite-strings -Werror

DEPS := libxml-2.0 json librtmp
CFLAGS += $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --cflags $(DEPS)) \
	-fPIC $(EXTRA_CFLAGS) $(WARNINGS)
LDFLAGS += -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME) $(EXTRA_LDFLAGS)
LDLIBS := $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --libs $(DEPS))

SRC := get_xml_buffer.c \
	get_config.c \
	get_index.c \
	parse_index.c \
	get_series_items.c \
	parse_series_items.c \
	destroy_config.c \
	destroy_index.c \
	get_auth.c \
	generate_video_uri.c \
	fetch_video.c \
	destroy_series_items.c \
	destroy_auth.c \
	wiicode/http.c \
	strtrim.c \
	find_series.c \
	find_item.c \
	easy_config.c \
	easy_index.c

OBJ := $(SRC:.c=.o)

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

.PHONY: clean
