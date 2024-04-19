#COMPILER:=$(CROSS_COMPILER)gcc
ifeq ($(CROSS_COMPILE),)
    COMPILER := gcc  # Default compiler is gcc
else
    COMPILER := $(CROSS_COMPILE)gcc
endif

all: writer

writer: writer.c
	$(COMPILER) -g -Wall -o writer writer.c

clean:
	@rm -f writer
