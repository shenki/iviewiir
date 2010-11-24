IVIEWIIR_SRC := iviewiir.c xdg-user-dir-lookup.c

IVIEWIIR_OBJ := $(IVIEWIIR_SRC:.c=.o)

IVIEWIIR_LIB := -lneon -ljson -lrtmp

LIBIVIEW := libiview/libiview.a

PROGRAM := iviewiir

CFLAGS := -Wall -Wextra -Wwrite-strings -Werror
LDFLAGS :=

all: $(PROGRAM)

$(PROGRAM): $(IVIEWIIR_OBJ) $(LIBIVIEW)
	$(CC) $(LDFLAGS) $^ $> -o $@  $(IVIEWIIR_LIB)

$(LIBIVIEW):
	@cd libiview; $(MAKE) libiview.a

%.o: %.c
	$(COMPILE.c) -MMD -MF $(subst .o,.d,$@) $(OUTPUT_OPTION) $<

clean:
	$(RM) $(IVIEWIIR_OBJ) $(IVIEWIIR_OBJ:.o=.d) $(PROGRAM)
	@cd libiview; $(MAKE) clean

.PHONY: clean
