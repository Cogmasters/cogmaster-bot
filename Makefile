CC ?= gcc

LIBDIR           = lib
INTERACTIONS_DIR = interactions

MAIN = main

CFLAGS  = -pthread -Wall -Wextra -O0 -g -I$(INTERACTIONS_DIR)
LDFLAGS = -L$(LIBDIR)
LDLIBS  = -ldiscord -lcurl

ARLIB = $(LIBDIR)/libcogmaster.a

all: $(MAIN)

$(MAIN): $(ARLIB)

$(ARLIB):
	@ $(MAKE) -C $(INTERACTIONS_DIR)

echo:
	@ echo -e 'CC: $(CC)\n'
	@ echo -e 'CFLAGS: $(CFLAGS)\n'
	@ echo -e 'MAIN: $(MAIN)\n'

clean:
	@ rm -f $(MAIN)
	@ $(MAKE) -C $(INTERACTIONS_DIR) clean

.PHONY: echo clean
