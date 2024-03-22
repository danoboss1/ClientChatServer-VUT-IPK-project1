.PHONY: all clean

CFLAGS := -g #-Wall -Wextra -pedantic -Wconversion -Wno-unused-parameter
PROG_OBJS := program.o client.o
TESTS_OBJS := client.o message_tests.o

all: ipk24chat-client

%.o: %.c
	$(CC) ${CFLAGS} -c -o $@ $<

ipk24chat-client: ${PROG_OBJS}
	$(CC) ${CFLAGS} -o $@ $^

tests: ${TESTS_OBJS}
	$(CC) ${CFLAGS} -o $@ $^

clean:
	rm -f ${PROG_OBJS} ${TESTS_OBJS} ipk24chat-client tests