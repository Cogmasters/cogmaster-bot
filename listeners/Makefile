CC ?= gcc
SRC := cmd.c menu.c
SETTERS := $(SRC:%.c=%)

CFLAGS := -Wall -Wextra -O0 -g -pthread
LDFLAGS := -ldiscord -lcurl -lcrypto -lm

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

all: $(SETTERS)

echo:
	@ echo SRC: $(SRC)
	@ echo SETTERS: $(SETTERS)

clean:
	rm $(SETTERS)
