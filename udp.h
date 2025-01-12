#ifndef CLIENT_H
#define CLIENT_H

#include "message_structure.h"

#include <stdlib.h>
#include <netinet/in.h>


size_t find_null_character_position(const char *str, size_t max_length);

Message receiveMessage(int sockfd, struct sockaddr_in* server_addr);

void Handle_message_from_server(const char *buffer, Message *msg);

void udp_main(struct sockaddr_in servaddr, int server_port, int udp_main, int max_retransmissions);

#endif