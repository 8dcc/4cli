
CC       := gcc
CFLAGS   := -std=c99 -Wall -Wextra -Wpedantic -Wshadow# -ggdb3 -fsanitize=address,leak,undefined -fstack-protector-strong
CPPFLAGS := -DUSE_COLOR
LDLIBS   := -lcurl -lcjson

SRC := main.c util.c request.c thread.c pretty.c
OBJ := $(addprefix obj/, $(addsuffix .o, $(SRC)))

BIN := 4cli

PREFIX := /usr/local
BINDIR := $(PREFIX)/bin

#-------------------------------------------------------------------------------

.PHONY: all clean install

all: $(BIN)

clean:
	rm -f $(OBJ)
	rm -f $(BIN)

install: $(BIN)
	install -D -m 755 $^ -t $(DESTDIR)$(BINDIR)

#-------------------------------------------------------------------------------

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<
