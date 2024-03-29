

#include <stdlib.h>
#include <netinet/in.h>

void tcp_main(struct sockaddr_in servaddr, int server_port);

void Handle_user_input_tcp(char *input_line, char *line_to_send_from_client, int sockfd);