#COMPILER:=$(CROSS_COMPILER)gcc

CFLAGS = -g -Wall

ifeq ($(CROSS_COMPILE),)
    COMPILER := gcc  # Default compiler is gcc
else
    COMPILER := $(CROSS_COMPILE)gcc
endif

all: writer

writer: writer.c
	$(COMPILER) $(CFLAGS) -o writer writer.c

clean:
	@rm -f writer
	@rm -f *.o
