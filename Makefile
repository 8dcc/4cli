
CC=gcc
CFLAGS=-Wall -Wextra
LDFLAGS=-lcurl

OBJ_FILES=main.c.o util.c.o dependencies/cJSON/cJSON.c.o
OBJS=$(addprefix obj/, $(OBJ_FILES))

BIN=4cli.out

.PHONY: clean all run

# -------------------------------------------

all: $(BIN)

run: $(BIN)
	./$<

clean:
	rm -f $(OBJS)
	rm -f $(BIN)

# -------------------------------------------

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

obj/%.c.o : src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<
