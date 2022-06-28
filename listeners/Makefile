CC ?= gcc

EXES = cmd menu

CFLAGS  = -pthread -Wall -Wextra -O0 -g
LDLIBS  = -ldiscord -lcurl

all: $(EXES)

echo:
	@ echo -e 'CC: $(CC)\n'
	@ echo -e 'CFLAGS: $(CFLAGS)\n'
	@ echo -e 'EXES: $(EXES)\n'

clean:
	@ rm -f $(EXES)

.PHONY: echo clean
