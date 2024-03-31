#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <poll.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "tcp.h"

// The main client function for the IPK24 Chat Server
int main(int argc, char *argv[]){

    // Variables for arguments processing
    int opt;
    bool is_protocol;
    bool is_ip_adress;

    bool entered_server_port = false;
    bool entered_udp_timeout = false;
    bool entered_max_retransmissions = false;

    char *protocol = NULL;
    char *server_address = NULL;
    uint16_t server_port; 
    uint16_t udp_timeout; 
    uint8_t max_retransmissions;

    unsigned long ul;
    char *endptr;
    
    while((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
    {
        switch(opt)
        {
            // Transport protocol variant
            case 't':
                if (strcmp(optarg, "udp") == 0 || (strcmp(optarg, "tcp")) == 0){
                    protocol = optarg;
                    is_protocol = true;
                } else {
                    return 0;
                }
                break;
            // Server IP/hostname
            case 's':
                server_address = optarg;
                is_ip_adress = true;
                break;
            // Port
            case 'p':
                ul = strtoul(optarg, &endptr, 10);

                // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found in the port value\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint16_t
                if (ul > UINT16_MAX) {
                    fprintf(stderr, "Port value out of range of uint16_t\n");
                    exit(EXIT_FAILURE);
                }

                server_port = (uint16_t)ul;
                entered_server_port = true;
                break;
            // UDP confirmation timeout
            case 'd':
                ul = strtoul(optarg, &endptr, 10);

                // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found in the UDP confirmation timeout\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint16_t
                if (ul > UINT16_MAX) {
                    fprintf(stderr, "UDP confirmation timeout value out of range of uint16_t\n");
                    exit(EXIT_FAILURE);
                }

                udp_timeout = (uint16_t)ul;
                entered_udp_timeout = true;
                break;
            // Maximum number of UDP retransmissions
            case 'r':
                ul = strtoul(optarg, &endptr, 10); 

                // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found in the max UDP retransmissions\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint8_t
                if (ul > UINT8_MAX) {
                    fprintf(stderr, "Max UDP retransmission value out of range of uint8_t\n");
                    exit(EXIT_FAILURE);
                }

                max_retransmissions = (uint8_t)ul;
                entered_max_retransmissions = true;
                break;
            // Prints program help output
            case 'h':
                printf("Program Help Output:\n");
                printf("-t: Transport protocol used for connection (tcp or udp)\n");
                printf("-s: Server IP or hostname\n");
                printf("-p: Server port (uint16)\n");
                printf("-d: UDP confirmation timeout (uint16)\n");
                printf("-r: Maximum number of UDP retransmissions (uint8)\n");
                printf("-h: Prints program help output and exits\n"); 
                exit(0);        
        }
    }

    if (is_protocol == false || is_ip_adress == false){
        return 0;
        fprintf(stderr, "Transport protocol(-t) and Server IP/hostname(-s) are mandatory parameters\n");
        fprintf(stderr, "U have to insert both of them\n");
        fprintf(stderr, "Use -h for help\n");
        exit(1);
    }

    // Setting default values for parameters
    if (entered_server_port == false) {
        server_port = 4567;
    }

    if (entered_udp_timeout == false) {
        udp_timeout = 250;
    }

    if (entered_max_retransmissions == false) {
        max_retransmissions = 3;
    }

    // Retrieves the host information corresponding to the given Server IP/hostname
    struct hostent *server = gethostbyname(server_address);
    if (server == NULL){
        fprintf(stderr, "ERROR: no such host %s\n", server_address);
        exit(EXIT_FAILURE);
    }

    // Structure to hold the server's address
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    memcpy(&servaddr.sin_addr.s_addr, server->h_addr_list[0], (size_t)server->h_length);

    // Run different transport protocols 
    if (strcmp(protocol, "udp") == 0) {
        printf("Max retransmission is %d\n", max_retransmissions);
        printf("Udp timeout is %d\n", udp_timeout);
        printf("UDP WASNT IMPLEMENTED CORRECTLY, SO I DID NOT INCLUDE IT\n");
        exit(0);
        //udp_main(servaddr, server_port, udp_timeout, max_retransmissions);
    } else if (strcmp(protocol, "tcp") == 0) {
        tcp_main(servaddr, server_port);
    }
}