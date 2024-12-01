CC:=gcc

CFLAGS:=-g -O0 -Wall -std=c17 -fsanitize=undefined -fsanitize=address
LDFLAGS:=-lm -laocaux
BUILD_MODE:=DEBUG

ifdef release
CFLAGS:=-O2 -Wall -std=c17 -DNDEBUG
BUILD_MODE:=RELEASE
endif

DIRS:=bin
DAYS:=$(wildcard day*)
TARGETS:=$(DAYS:%=bin/%)

all: $(TARGETS)

bin/%: %/main.c | $(DIRS)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@ $(LDFLAGS)

$(DAYS): %: bin/%

-include $(OBJS:.o=.d)

$(DIRS):
	mkdir -p $@

clean:
	rm -rf bin

.PHONY: all clean $(DAYS)

