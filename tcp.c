#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <poll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <regex.h>

#include "tcp.h"
#include "message_structure.h"


void tcp_main(struct sockaddr_in servaddr, int server_port) {
    char input_line[FULL_MESSAGE_BUFFER + 1];

    char line_to_send_from_client[FULL_MESSAGE_BUFFER + 1];

    char received_message[FULL_MESSAGE_BUFFER + 1];

    State current_state = START_STATE;

    MessageType msg_type;

    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR: opening socket");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("ERROR: connecting");
        exit(EXIT_FAILURE);
    }

    // Define pollfd structures
    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // For stdin
    fds[0].events = POLLIN;
    fds[1].fd = sockfd; // For socket
    fds[1].events = POLLIN;

    while(true){ 
        // tu bude nejaky stavovy podla zadania
        // do terminalu pisem rovnako, len serveru posielam iny format, aj on mne posiela v inom formate

        // // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout

        char *AUTH_COMMAND_TO_CHECK = "/auth";

        // nieco mi prislo na standartny vstup
        if (fds[0].revents & POLLIN) {
            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

            if (current_state == START_STATE){

                // display name musim ukladat, ale az ked mi to server prijme
            
            // AUTH PREHADZUJEM PRIDE MENO,SECRET,DNAME A POSIELAM MENO DNAME SECRET
            if (strncmp(input_line, AUTH_COMMAND_TO_CHECK, strlen(AUTH_COMMAND_TO_CHECK)) == 0) {
                char delete_auth_zaciatok[5] = "";
                char username[USERNAME_MAX_LENGTH + 1] = "";
                char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";
                char secret[SECRET_MAX_LENGHT + 1] = "";

                // tato FUNKCIA MI TO ROZDELI DO PREMENNYCH
                sscanf(input_line, "%s %s %s %s", delete_auth_zaciatok, username, secret, display_name);

                sprintf(line_to_send_from_client, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);
                size_t message_size  = strlen(line_to_send_from_client);

                ssize_t bytes_sent;

                // Use write() function to send data
                bytes_sent = write(sockfd, line_to_send_from_client, message_size);
                if (bytes_sent < 0) {
                    perror("ERROR: sending message");
                    exit(EXIT_FAILURE);
                }
                // write a read
                // vlozim do prazdneho bufferu spravu a zistim aky velky je ten buffer

                current_state = AUTH_STATE;

                // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
            }
        }
    }
    // /r/n bude v tom bufferi, ktory posielam aj v bufferi ktory prijimam
    
    /*
        // Variable to create regex
        regex_t reegex;
    
        // Variable to store the return
        // value after creation of regex
        int value;
    
        // Function call to create regex
        value = regcomp( &reegex, "[:word:]", 0);
    
        // If compilation is successful
        if (value == 0) {
            printf("RegEx compiled successfully.");
        }
    
        // Else for Compilation error
        else {
            printf("Compilation error.");
        }
        return 0;
    */


    // Don't forget to close the socket when done
        //TU SPRACOVAVAM SPRAVY OD SERVERU
        if (fds[1].revents & POLLIN) {
            ssize_t bytes_received;

            // Use read() function to receive data
            bytes_received = read(sockfd, received_message, sizeof(received_message));
            if (bytes_received < 0) {
                perror("ERROR: receiving message");
                exit(EXIT_FAILURE);
            }

            // tu spracujem spravu a zistim aky je to typ
            // message_type = 'ERR';

            if (current_state == AUTH_STATE){
                if (msg_type == MESSAGE_TYPE_NOT_REPLY) {
                    current_state = AUTH_STATE;
                } else if (msg_type == MESSAGE_TYPE_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_ERR) {
                    // SendBye od uzivatela este
                    current_state = END_STATE;
                } else {
                    // SendErr od uzivatela este
                    current_state = ERROR_STATE;
                    // SendBye od uzivatela este
                    // A IDES DO END_STATE
                }
            } else if (current_state == OPEN_STATE) {
                if (msg_type == MESSAGE_TYPE_NOT_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_ERR) {
                    // SendBye od uzivatela este
                    current_state = END_STATE;
                } else if (msg_type == MESSAGE_TYPE_BYE) {
                    current_state = END_STATE;
                } else {
                    // SendErr od uzivatela este
                    current_state = ERROR_STATE;
                    // SendBye od uzivatela este
                    // A IDES DO END_STATE
                }
            }

        }
    close(sockfd);
    }
}