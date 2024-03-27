#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <poll.h>
#include <getopt.h>

#include <sys/socket.h>
#include <arpa/inet.h>

// client je udp
#include "client.h"

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

    char *protocol = NULL;
    char *server_address = NULL;
    uint16_t server_port = 0; 
    uint16_t udp_timeout = 0; 
    uint8_t max_retransmissions = 0;
    
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
            case 's':
                server_address = optarg;
                is_ip_adress = true;
            case 'p':
                // Parse the server port value provided by the user
                server_port = atoi(optarg);
            case 'd':
                // Parse the UDP confirmation timeout value provided by the user
                udp_timeout = atoi(optarg);
            case 'r':
                max_retransmissions = atoi(optarg);
            case 'h':
                // Print program help output and exit
                printf("Program Help Output:\n");
                printf("-t: Transport protocol used for connection (tcp or udp)\n");
                printf("-s: Server IP or hostname\n");
                printf("-p: Server port (uint16)\n");
                printf("-d: UDP confirmation timeout (uint16)\n");
                printf("-r: Maximum number of UDP retransmissions (uint8)\n");
                printf("-h: Prints program help output and exits\n");         
        }
    }

    if (is_protocol == false || is_ip_adress == false){
        return 0;
    }


    // struct hostent *server = gethostbyname(ip_add);
    // if (server == NULL){
    //     fprintf(stderr, "ERROR: no such host %s\n", ip_add);
    //     exit(EXIT_FAILURE);
    // }

    // struct sockaddr_in servaddr;
    // memset(&servaddr, 0, sizeof(servaddr));

    // servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(server_port);
    // memcpy(&servaddr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length);

    udp_main();


}