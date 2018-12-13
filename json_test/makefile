src = $(wildcard *.c)
targets = $(patsubst %.c, %, $(src))

CC = gcc
CFLAGS = cJSON.cZZ

all:$(targets)

$(targets):%:%.c
		$(CC) $< -o $@ $(CFLAGS)

.PHONY:clean all
	clean:
		-rm -rf $(targets) 
