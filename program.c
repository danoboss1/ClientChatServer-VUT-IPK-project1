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

// client je udp
#include "client.h"
#include "tcp.h"
// #include "message_structure.h"


//TODO: IFDEF PRI HEADER SUBOROCH

//TODO2: 
// to dalsieho suboru si mozem definovat vsetky struktury a tak
// definujes struktury na vsetky rozne typy
// dostanes spravy type a id vies aku ma velkost
// toto vsetko v 1500 dlhom buffery
// a potom si este definujes max_lenghy 
// podla max_lengu jednotlivych typov v contente posielas do fukncie, ktora ti zisti dlzku retazca daneho typu, toto cez ukazatele
// a potom sa posunies o konkretnu dlzku v skutocnom kontexte

//TODO3:
// vycistit si kod od nepotrebneho bordelu
// zistit co vsetko ako funguje a okomentovat si to 
// loop na spravy
// parameters handle
// rozdelenie spravy na tie jednotlive veci
// specifikacie vymienania sprav
// logika stavoveho automatu
// tuto mozem naimplementovat cele handlovanie parametrov


void process_arguments(int argc, char *argv[]){
    if(argc == 1){
        return;
    }  

    if(strcmp(argv[1], "-h") == 0) {
        printf("Usage:\n");
        printf("    Help: [-h]\n");
        printf("    Mandatory parameters: program_name -t <tcp|udp> -s <server_address>\n");
        printf("    Optional parameters: [-p <server_port>] [-d <udp_timeout>] [-r <max_retransmissions>]\n");
    }
}


int main(int argc, char *argv[]){

    // FUNKCIA NA HANDLOVANIE ARGUMENTOV PRI SPUSTENI PROGRAMU
    // process_arguments(argc, argv);

    int opt;
    bool is_protocol;
    bool is_ip_adress;
    bool entered_server_port = false;

    char *protocol = NULL;
    char *server_address = NULL;
    uint16_t server_port = 0; 
    uint16_t udp_timeout = 0; 
    uint8_t max_retransmissions = 0;

    unsigned long ul;
    char *endptr;
    
    while((opt = getopt(argc, argv, "t:s:p:d:r:h")) != -1)
    {
        switch(opt)
        {
            case 't':
                if (strcmp(optarg, "udp") == 0 || (strcmp(optarg, "tcp")) == 0){
                    protocol = optarg;
                    is_protocol = true;
                } else {
                    return 0;
                }
                break;
            case 's':
                server_address = optarg;
                is_ip_adress = true;
                break;
            case 'p':
                // Parse the server port value provided by the user
                ul = strtoul(optarg, &endptr, 10);

                    // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint16_t
                if (ul > UINT16_MAX) {
                    fprintf(stderr, "Value out of range of uint16_t\n");
                    exit(EXIT_FAILURE);
                }

                // Convert to uint16_t
                server_port = (uint16_t)ul;

                // server_port = atoi(optarg);
                entered_server_port = true;
                break;
            case 'd':
                // Parse the UDP confirmation timeout value provided by the user
                ul = strtoul(optarg, &endptr, 10);

                // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint16_t
                if (ul > UINT16_MAX) {
                    fprintf(stderr, "Value out of range of uint16_t\n");
                    exit(EXIT_FAILURE);
                }

                // Convert to uint16_t
                udp_timeout = (uint16_t)ul;
                // udp_timeout = atoi(optarg);
                break;
            case 'r':
                ul = strtoul(optarg, &endptr, 10); // Base 10 conversion

                // Check for errors during conversion
                if ((errno == ERANGE && (ul == ULONG_MAX || ul == 0)) || (errno != 0 && ul == 0)) {
                    perror("strtoul");
                    exit(EXIT_FAILURE);
                }

                // Check if there are any non-numeric characters in the string
                if (endptr == optarg) {
                    fprintf(stderr, "No digits were found\n");
                    exit(EXIT_FAILURE);
                }

                // Check if the value fits within the range of uint8_t
                if (ul > UINT8_MAX) {
                    fprintf(stderr, "Value out of range of uint8_t\n");
                    exit(EXIT_FAILURE);
                }

                // Convert to uint8_t
                max_retransmissions = (uint8_t)ul;
                // max_retransmissions = atoi(optarg);
                break;
            case 'h':
                // Print program help output and exit
                printf("Program Help Output:\n");
                printf("-t: Transport protocol used for connection (tcp or udp)\n");
                printf("-s: Server IP or hostname\n");
                printf("-p: Server port (uint16)\n");
                printf("-d: UDP confirmation timeout (uint16)\n");
                printf("-r: Maximum number of UDP retransmissions (uint8)\n");
                printf("-h: Prints program help output and exits\n"); 
                break;        
        }
    }

    if (is_protocol == false || is_ip_adress == false){
        return 0;
    }

    if (entered_server_port == false) {
        server_port = 4567;
    }


    struct hostent *server = gethostbyname(server_address);
    if (server == NULL){
        fprintf(stderr, "ERROR: no such host %s\n", server_address);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    memcpy(&servaddr.sin_addr.s_addr, server->h_addr_list[0], (size_t)server->h_length);

    // tuto budem rozlisovat tcp alebo udp podla parametru zadaneho z terminalu
    if (strcmp(protocol, "udp") == 0) {
        udp_main(servaddr, server_port);
    } else if (strcmp(protocol, "tcp") == 0) {
        tcp_main(servaddr, server_port);
    }
}