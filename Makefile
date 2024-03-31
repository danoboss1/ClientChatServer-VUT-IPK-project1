.PHONY: all clean

CFLAGS := -g -Wall -Wextra -pedantic -Wconversion -Wno-unused-parameter
PROG_OBJS := program.o udp.o tcp.o
UDP_TESTS_OBJS := udp.o tests/udp_message_tests.o 
TCP_SERVER_TESTS_OBJS := tcp.o tests/tcp_server_messages_tests.o

all: ipk24chat-client

%.o: %.c
	$(CC) ${CFLAGS} -c -o $@ $<

ipk24chat-client: ${PROG_OBJS}
	$(CC) ${CFLAGS} -o $@ $^

udp_test: ${TESTS_OBJS}
	$(CC) ${CFLAGS} -o $@ $^

tcp_server_test: ${TCP_SERVER_TESTS_OBJS}
	$(CC) ${CFLAGS} -o $@ $^

clean:
	rm -f ${PROG_OBJS} ${TESTS_OBJS} ${TCP_SERVER_TESTS_OBJS} ipk24chat-client udp_test tcp_server_test