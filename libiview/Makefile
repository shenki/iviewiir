SONAME := libiview.so

# Uncomment when building for the wii, or provide on the command line
# CROSS_COMPILE=powerpc-eabi-

ifeq ($(CROSS_COMPILE),powerpc-eabi-)
PKGCONFIG := /opt/devkitpro/portlibs/ppc/lib/pkgconfig
EXTRA_CFLAGS := -O3 -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DHW_RVL \
		-I/opt/devkitpro/libogc/include	\
		-I/opt/devkitpro/libogc/lib/wii
EXTRA_LDFLAGS = -L/opt/devkitpro/devkitPPC/powerpc-eabi/lib \
		-L/opt/devkitpro/libogc/lib/wii
endif

CC	:=$(CROSS_COMPILE)$(CC)
AR	:=$(CROSS_COMPILE)ar

DEPS := libxml-2.0 json librtmp
CFLAGS := $(shell PKG_CONFIG_PATH=$(PKGCONFIG) pkg-config --cflags $(DEPS)) \
	-fPIC  $(EXTRA_CFLAGS)
LDFLAGS := -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME) $(EXTRA_LDFLAGS)
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
	wiicode/http.c

OBJ := $(SRC:.c=.o)

CFLAGS := $(CFLAGS) $(CFLAGS) -Wall -Wextra -Wwrite-strings -Werror
LDFLAGS := $(LDFLAGS)

all: libiview.so libiview.a

libiview.so: $(OBJ)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $> $(LDLIBS)

static: libiview.a

libiview.a: $(OBJ)
	$(AR) rs $@ $?

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJ) $(OBJ:.o=.d) libiview.so libiview.a

.PHONY: clean
