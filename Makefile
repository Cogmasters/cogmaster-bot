CC ?= gcc

INTERACTIONSDIR := interactions
OBJDIR := obj

SRC := $(INTERACTIONSDIR)/utils.c $(wildcard $(INTERACTIONSDIR)/**/*.c)
OBJS := $(SRC:$(INTERACTIONSDIR)/%.c=$(OBJDIR)/%.o)

MAIN := main

CFLAGS := -pthread -Wall -Wextra -O0 -g -I$(INTERACTIONSDIR)
LDFLAGS := -ldiscord -lcurl

$(OBJDIR)/%.o: $(INTERACTIONSDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(MAIN)

$(MAIN): $(MAIN).c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(dir $(OBJS))

echo:
	@ echo SRC: $(SRC)
	@ echo OBJS: $(OBJS)

clean:
	rm -rf $(MAIN)
	rm -rf $(OBJDIR)
