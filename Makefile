IVIEWIIR_SRC := iviewiir.c xdg-user-dir-lookup.c ccan/opt/helpers.c \
		ccan/opt/usage.c ccan/opt/opt.c ccan/opt/parse.c

IVIEWIIR_OBJ := $(IVIEWIIR_SRC:.c=.o)

IVIEWIIR_LIB := -lxml2 -ljson -lrtmp

LIBIVIEW := libiview/libiview.a

PROGRAM := iviewiir

WARNINGS := -Wall -Wextra -Wwrite-strings -Werror
CFLAGS := $(CFLAGS) $(shell pkg-config --cflags libxml-2.0) $(WARNINGS) -I.
LDFLAGS :=

all: $(PROGRAM)

$(PROGRAM): $(IVIEWIIR_OBJ) $(LIBIVIEW)
	$(CC) $(LDFLAGS) $^ $> -o $@  $(IVIEWIIR_LIB)

# Hack to ensure recursive call is made
FORCE:

$(LIBIVIEW): FORCE
	@cd libiview; $(MAKE) libiview.a

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(IVIEWIIR_OBJ) $(IVIEWIIR_OBJ:.o=.d) $(PROGRAM)
	@cd libiview; $(MAKE) clean

.PHONY: clean
