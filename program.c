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

    // puts("Neposlal som nikde nic");
    while(true){ 
        // tu bude nejaky stavovy podla zadania

        // // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout

        // TOTO JE NA CITANIE JEDNEHO RIADKU Z STANDARTNEHO VSTUPU = SPRAVA CO POSIELAM SERVERU
        fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);


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