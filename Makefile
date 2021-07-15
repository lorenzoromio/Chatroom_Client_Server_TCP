CC = gcc -Wall -pedantic -std=gnu99
HEADERS := $(shell find ./src -type f -name *.h)
COMMON := $(wildcard ./src/log/*.c)  $(wildcard ./src/utils/*.c)
COMMON := $(patsubst %.c, %.o, $(COMMON))
SERVER_OBJ := $(COMMON) $(patsubst ./src/server/%.c, ./obj/server/%.o, $(wildcard ./src/server/*.c))
CLIENT_OBJ := $(COMMON) $(patsubst ./src/client/%.c, ./obj/client/%.o, $(wildcard ./src/client/*.c))

.PHONY: all clean

all: server client

server: $(SERVER_OBJ)
	$(CC) $^ -o $@ -lpthread

client: $(CLIENT_OBJ)
	$(CC) $^ -o $@ -lpthread

obj/%.o: src/%.c $(HEADERS)
	@mkdir -p $(@D)
	$(CC) -c $< -o $@

clean:
	rm -f $(SERVER_OBJ) $(CLIENT_OBJ) ./server ./client
