NATIVE_CC=gcc

ifndef CROSS_COMPILER
	override COMPILER=$(NATIVE_CC)
else
	override COMPILER=CROSS_COMPILER 
endif
writer: writer.c
	$(COMPILER) -g -Wall -o writer writer.c

clean:
	@rm -f writer
