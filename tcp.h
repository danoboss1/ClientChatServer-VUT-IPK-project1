#ifndef TCP_H
#define TCP_H

#include "message_structure.h"

#include <stdlib.h>
#include <netinet/in.h>


void tcp_main(struct sockaddr_in servaddr, int server_port);

MessageType Handle_user_input_tcp(char *input_line, char *line_to_send_from_client, int sockfd, State current_state);

MessageType Handle_server_messages_tcp(char *received_message);

void tcp_handler (int signum);

void user_BYE(char *line_to_send_from_client, int sockfd);

#endif