SONAME := libiview.so

DEPS := libxml-2.0 json librtmp
CFLAGS := -fPIC $(shell pkg-config --cflags $(DEPS))
LDFLAGS := -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME)
LIBIVEW_LDLIBS := $(shell pkg-config --libs $(DEPS))

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

libiview.a: $(OBJ)
	$(AR) rs $@ $?

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(OBJ) $(OBJ:.o=.d) libiview.so libiview.a

.PHONY: clean
