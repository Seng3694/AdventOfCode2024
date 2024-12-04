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

bin/%: %/main.c %/input.txt | $(DIRS)
	$(CC) $(CFLAGS) -MMD -MP $< -o $@ $(LDFLAGS) \
		-D INPUT_HEIGHT=$(shell wc -l < $*/input.txt) \
		-D INPUT_WIDTH=$(shell wc -L < $*/input.txt | awk '{print $1}')

$(DAYS): %: bin/%

-include $(OBJS:.o=.d)

$(DIRS):
	mkdir -p $@

clean:
	rm -rf bin

.PHONY: all clean $(DAYS)

