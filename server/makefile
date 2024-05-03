
CFLAGS = -g -Wall

ifeq ($(CROSS_COMPILE),)
    COMPILER := gcc  # Default compiler is gcc
else
    COMPILER := $(CROSS_COMPILE)gcc
endif

CC = $(COMPILER)

aesdsocket : main.c
	$(CC) -g -o aesdsocket main.c


.PHONY: clean
clean:
	@rm -f aesdsocket
