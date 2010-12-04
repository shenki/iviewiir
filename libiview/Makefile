SONAME := libiview.so

LIBIVIEW_DEPS := neon json librtmp
LIBIVIEW_CFLAGS := -fPIC
LIBIVIEW_LDFLAGS := -Wl,-Map,$(notdir $@).map -Wl,-soname,$(SONAME)
LIBIVEW_LDLIBS := $(shell pkg-config --libs $(LIBIVIEW_DEPS))
LIBIVIEW_INC := 

LIBIVIEW_SRC := get_xml_buffer.c \
	parse_config.c \
	get_index.c \
	parse_index.c \
	get_series_items.c \
	parse_series_items.c \
	destroy_config.c \
	destroy_index.c \
	parse_auth.c \
	generate_video_uri.c \
	fetch_video.c \
	destroy_series_items.c \
	destroy_auth.c

LIBIVIEW_OBJ := $(LIBIVIEW_SRC:.c=.o)

CFLAGS := $(CFLAGS) $(LIBIVIEW_CFLAGS) -Wall -Wextra -Wwrite-strings -Werror
LDFLAGS := $(LIBIVIEW_LDFLAGS)

all: libiview.so libiview.a

libiview.so: $(LIBIVIEW_OBJ)
	$(CC) -shared $(LDFLAGS) -o $@ $^ $> $(LIBIVIEW_LDLIBS)

libiview.a: $(LIBIVIEW_OBJ)
	$(AR) rs $@ $?

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(LIBIVIEW_OBJ) $(LIBIVIEW_OBJ:.o=.d) libiview.so libiview.a

.PHONY: clean
