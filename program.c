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
    // Allocate memory for the test_buffer_for_max_length_function
    char *test_buffer_for_max_length_function = (char *)malloc(50 * sizeof(char)); // Allocate memory for a test_buffer_for_max_length_function of size 50


    // Populate the test_buffer_for_max_length_function with content
    strcpy(test_buffer_for_max_length_function, "Hello, world!"); // Copy the string into the test_buffer_for_max_length_function

    // Call the find_null_character_position function with the test_buffer_for_max_length_function and the maximum length to consider
    size_t length = find_null_character_position(test_buffer_for_max_length_function, 50);

    // Output the length until the first null character
    // printf("Length until the first null character: %zu\n", length);

    // Free the allocated memory for the test_buffer_for_max_length_function
    free(test_buffer_for_max_length_function);

    // printf("Program sa uspesne spusti");

    process_arguments(argc, argv);

    // printf("coje");

    // connect_socket();

    // char buffer[MAXLINE];
    // char *message = "Hello Server";
    char input_line[FULL_MESSAGE_BUFFER + 1];

    char line_to_send_from_client[FULL_MESSAGE_BUFFER + 1];

    // TOTO MOZNO DAT NASPAT
    // fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

    struct sockaddr_in servaddr;

    // Clear servaddr
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    // puts("Nikde som nic nevytvoril");

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

    // puts("Neposlal som nikde nic");
    while(true){ 
        // tu bude nejaky stavovy podla zadania

        // // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout

        // toto je posielanie AUTENTIFIKACIE ZATIAL IBA
        // nieco mi prislo na standartny vstup
        // toto je trochu zle lebo ta sprava vyzera trochu inak
        if (fds[0].revents & POLLIN) {

            // TOTO JE NA CITANIE JEDNEHO RIADKU Z STANDARTNEHO VSTUPU = SPRAVA CO POSIELAM SERVERU
            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

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

            size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + username_length + display_name_length + secret_length + 1;  

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

            if (sendto(client_socket, line_to_send_from_client, strlen(input_line), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Error: sendto failed");
            close(client_socket);
            exit(EXIT_FAILURE);
            }

            msg_id++;

            // // tato funkcia mi to rozdeli do tej struktury a ja to musim este nejako zlozit do bufferu, ktory poslem serveru
            // Handle_message_from_server(input_line, &client_message);

            // // Populate buffer
            // line_to_send_from_client[0] = client_message.type;
            // memcpy(line_to_send_from_client + 1, &client_message.messageID, sizeof(uint16_t));

            // size_t username_length = strlen(client_message.data.auth.username);
            // size_t display_name_length = strlen(client_message.data.auth.displayName);
            // size_t secret_length = strlen(client_message.data.auth.secret);

            // size_t offset = sizeof(uint8_t) + sizeof(uint16_t);

            // memcpy(line_to_send_from_client + offset, client_message.data.auth.username, username_length);
            // line_to_send_from_client[offset + username_length] = '\0'; // Null-terminate message contents
            // offset += username_length + 1;

            // memcpy(line_to_send_from_client + offset, client_message.data.auth.displayName, display_name_length);
            // line_to_send_from_client[offset + display_name_length] = '\0'; // Null-terminate message contents
            // offset += display_name_length + 1;

            // memcpy(line_to_send_from_client + offset, client_message.data.auth.secret, secret_length);
            // line_to_send_from_client[offset + secret_length] = '\0'; // Null-terminate message contents
            // offset += secret_length + 1;
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

        // CLIENT POSIELA IBA BUFFER 
        // Send message to the server
        if (sendto(client_socket, input_line, strlen(input_line), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("Error: sendto failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }

        // puts("Dostal som sa za sendto");

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