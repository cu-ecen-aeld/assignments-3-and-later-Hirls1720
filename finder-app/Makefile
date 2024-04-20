#COMPILER:=$(CROSS_COMPILER)gcc

CFLAGS = -g -Wall

ifeq ($(CROSS_COMPILE),)
    COMPILER := gcc  # Default compiler is gcc
else
    COMPILER := $(CROSS_COMPILE)gcc
endif

CC = $(COMPILER)

all: writer

writer: writer.c
	$(CC) $(CFLAGS) -o writer writer.c

clean:
	@rm -f writer
	@rm -f *.o
