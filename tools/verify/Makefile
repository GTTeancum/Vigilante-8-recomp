# tools/verify/Makefile -- builds the snapshot comparator.
#
# Host build only -- not compiled into the bit-exact target. The
# comparator is a developer tool used to diff a snapshot captured
# in an instrumented emulator against one captured in the port.

CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra -std=c11 -I../../include
LDFLAGS ?=

OBJS = state_snapshot.o input_recorder.o
TARGET = run_compare

all: $(TARGET)

$(TARGET): run_compare.o $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(TARGET)

.PHONY: all clean
