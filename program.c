#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <poll.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include "client.h"

uint16_t msg_id = 1;

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
    process_arguments(argc, argv);

    char input_line[FULL_MESSAGE_BUFFER + 1];

    char line_to_send_from_client[FULL_MESSAGE_BUFFER + 1];

    struct sockaddr_in servaddr;

    // Clear servaddr
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    // Create datagram socket
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int client_socket = socket(family, type, 0);
    if (client_socket <= 0) 
        {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
        }
    
    //bindovanie serveru na mna
    struct sockaddr_in bind_addr;
    memset(&bind_addr, 0, sizeof(struct sockaddr_in));
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_port = htons(PORT + 1);
    bind(client_socket, (struct sockaddr*)&bind_addr, sizeof(struct sockaddr_in));

    // Define pollfd structures
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // For stdin
    fds[0].events = POLLIN;
    fds[1].fd = client_socket; // For socket
    fds[1].events = POLLIN;

    Message client_message;
    Message server_message;

    while(true){ 
        // tu bude nejaky stavovy podla zadania

        // // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout

        // nieco mi prislo na standartny vstup
        if (fds[0].revents & POLLIN) {

            char *AUTH_COMMAND_TO_CHECK = "/auth";
            char *JOIN_COMMAND_TO_CHECK = "/join";
            char *RENAME_COMMAND_TO_CHECK = "/rename";
            char *HELP_COMMAND_TO_CHECK = "/help";

            // TOTO JE NA CITANIE JEDNEHO RIADKU Z STANDARTNEHO VSTUPU = SPRAVA CO POSIELAM SERVERU
            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

            if (strncmp(input_line, AUTH_COMMAND_TO_CHECK, strlen(AUTH_COMMAND_TO_CHECK)) == 0) {
                uint8_t type = 0x02;
                char delete_auth_zaciatok[5] = "";
                char username[USERNAME_MAX_LENGTH + 1] = "";
                char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";
                char secret[SECRET_MAX_LENGHT + 1] = "";

                // tato FUNKCIA MI TO ROZDELI DO PREMENNYCH
                sscanf(input_line, "%s %s %s %s", delete_auth_zaciatok, username, display_name, secret);

                size_t username_length = strlen(username);
                size_t display_name_length = strlen(display_name);
                // printf("%ld", display_name_length);
                size_t secret_length = strlen(secret);

                // 3 je pocet terminalnych nul, to asi dat neskor do nejakej premennej
                size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + username_length + display_name_length + secret_length + 3;  

                // Populate buffer
                uint8_t *ptr = (uint8_t *)line_to_send_from_client;
                *ptr++ = type;
                memcpy(ptr, &msg_id, sizeof(uint16_t));
                ptr += sizeof(uint16_t);
                strcpy(ptr, username);
                ptr += username_length + 1;
                strcpy(ptr, display_name);
                ptr += display_name_length + 1;
                strcpy(ptr, secret);

                //poslal som to co bolo aj na klavesnici plus nejake bajty navyse

                // treti parameter je dlzka co posielam
                if (sendto(client_socket, line_to_send_from_client, message_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Error: sendto failed");
                close(client_socket);
                exit(EXIT_FAILURE);
                }

                msg_id++;
            } else if (strncmp(input_line, JOIN_COMMAND_TO_CHECK, strlen(JOIN_COMMAND_TO_CHECK)) == 0) {
                uint8_t type = 0x03;
                char delete_join_zaciatok[5] = "";
                char channelID[CHANNEL_ID_MAX_LENGTH + 1] = "";
                char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";

                // tato FUNKCIA MI TO ROZDELI DO PREMENNYCH
                sscanf(input_line, "%s %s %s", delete_join_zaciatok, channelID, display_name);

                size_t channelID_length = strlen(channelID);
                size_t display_name_length = strlen(display_name);

                // 2 je pocet terminalnych nul, to asi data neskor do nejakej premennej
                size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + channelID_length + display_name_length + 2;  

                // Populate buffer
                uint8_t *ptr = (uint8_t *)line_to_send_from_client;
                *ptr++ = type;
                memcpy(ptr, &msg_id, sizeof(uint16_t));
                ptr += sizeof(uint16_t);
                strcpy(ptr, channelID);
                ptr += channelID_length + 1;
                strcpy(ptr, display_name);

                //poslal som to co bolo aj na klavesnici plus nejake bajty navyse

                if (sendto(client_socket, line_to_send_from_client, message_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Error: sendto failed");
                close(client_socket);
                exit(EXIT_FAILURE);
                }

                msg_id++;

            } else if (strncmp(input_line, RENAME_COMMAND_TO_CHECK, strlen(RENAME_COMMAND_TO_CHECK)) == 0) {
                puts("Locally changes the display name of the user to be sent with new messages/selected commands");
            } else if (strncmp(input_line, HELP_COMMAND_TO_CHECK, strlen(HELP_COMMAND_TO_CHECK)) == 0) {
                puts("Prints out supported local commands with their parameters and a description");
            }       
            else {
                // TOTO NEJAKO PREMENIT NA FINITE STATE MACHINE 
                // NECHAPEM PRECO TENTO ELSE NEJDE 
                printf("not recognize command!");
            }
        }


        // if (ret < 0) {
        //     perror("Poll error");
        //     exit(EXIT_FAILURE);
        // }

        // TU SPRACOVAVAM MOJE, KLIENTOVE SPRAVY/COMMANDY
        // // Check for events
        // if (fds[0].revents & POLLIN) {
        //     // Message from stdin
        //     char buffer[MAXLINE];
        //     fgets(buffer, MAXLINE, stdin);
        //     // Process and send message
        //     // ...
        // }

        // TU SPRACOVAVAM SPRAVY OD SERVERU
        // if (fds[1].revents & POLLIN) {
        //     // Message from server
        //     struct Message msg;
        //     recvfrom(sockfd, &msg, sizeof(msg), 0, NULL, NULL);
        //     // Process received message
        //     // ...
        // }


        // toto je na receiveMessage funkciu
        Message receivedMsg;
        receivedMsg = receiveMessage(client_socket, &servaddr);
    }


    // // Print the received response
    // printf("Server response: %s\n", buffer);

    // // Close the socket
    // close(client_socket);
    // return 0;
}