TOP = ..
CC ?= gcc

LIBDIR                 = $(TOP)/lib
SELECT_ROLES_DIR       = select-roles
RUBBERDUCK_CHANNEL_DIR = rubberduck-channel

OBJS = utils.o                               \
       $(SELECT_ROLES_DIR)/menu.o            \
       $(RUBBERDUCK_CHANNEL_DIR)/action.o    \
       $(RUBBERDUCK_CHANNEL_DIR)/configure.o \
       $(RUBBERDUCK_CHANNEL_DIR)/delete.o    \
       $(RUBBERDUCK_CHANNEL_DIR)/menu.o

CFLAGS = -Wall -Wextra -O0 -g -I.

ARLIB   = $(LIBDIR)/libcogmaster.a
ARFLAGS = -cqsv

all: $(ARLIB)

$(ARLIB): $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

echo:
	@ echo -e 'CC: $(CC)\n'
	@ echo -e 'CFLAGS: $(CFLAGS)\n'
	@ echo -e 'OBJS: $(OBJS)\n'

clean:
	@ rm -f $(OBJS)
	@ rm -rf $(LIBDIR)/*

.PHONY: echo clean
