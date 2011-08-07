IVIEWIIR_SRC := iviewiir.c xdg-user-dir-lookup.c ccan/opt/helpers.c \
		ccan/opt/usage.c ccan/opt/opt.c ccan/opt/parse.c

IVIEWIIR_OBJ := $(IVIEWIIR_SRC:.c=.o)

-include $(IVIEWIIR_OBJ:.o=.d)

IVIEWIIR_LIB := $(shell pkg-config --libs json librtmp libxml-2.0)
IVIEWIIR_LIB_STATIC := libjson.a librtmp.a libpolarssl.a libxml2.a -lz

LIBIVIEW := libiview/libiview.a
LIBFLVII := libflvii/libflvii.a

PROGRAM := iviewiir

WARNINGS := -Wall -Wextra -Wwrite-strings -Werror
CFLAGS := $(CFLAGS) $(shell pkg-config --cflags libxml-2.0) $(WARNINGS) -I.
LDFLAGS :=

all: $(PROGRAM)

$(PROGRAM): $(IVIEWIIR_OBJ) $(LIBIVIEW) $(LIBFLVII)
	$(CC) $(LDFLAGS) $^ $> -o $@  $(IVIEWIIR_LIB)

static: $(IVIEWIIR_OBJ) $(LIBIVIEW) $(LIBFLVII) nanohttp.o
	$(CC) $(LDFLAGS) $^ $> -o $(PROGRAM)  $(IVIEWIIR_LIB_STATIC)

# Hack to ensure recursive call is made
FORCE:

$(LIBIVIEW): FORCE
	@cd libiview; $(MAKE) libiview.a

$(LIBFLVII): FORCE
	@cd libflvii; $(MAKE) libflvii.a

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(IVIEWIIR_OBJ) $(IVIEWIIR_OBJ:.o=.d) $(PROGRAM)
	@cd libiview; $(MAKE) clean
	@cd libflvii; $(MAKE) clean

.PHONY: clean
