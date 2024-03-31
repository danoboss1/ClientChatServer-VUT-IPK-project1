#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "udp.h"


// First message id
uint16_t msg_id = 1;

// Function that find where to divide bigger blocks of the message to smaller necessary parts
size_t find_null_character_position(const char *str, size_t max_length) {

    size_t length = strnlen(str, max_length); 
    return length;
}

// Function to receive a message
Message receiveMessage(int sockfd, struct sockaddr_in* server_addr) {

    Message msg;
    socklen_t len = sizeof(*server_addr);

    recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)server_addr, &len);
    return msg;
}

// Splits the message from server to logical parts and stores them into Message structure
void Handle_message_from_server(const char *buffer, Message *msg){

    memcpy(&(msg->type),buffer, sizeof(uint8_t));  // Message Type
    memcpy(&(msg->messageID), buffer+1, sizeof(uint16_t));  // Message ID

    size_t offset = sizeof(uint8_t) + sizeof(uint16_t); // Overall offset in message (now type + Message ID) 
    size_t content_length;  // Offset for individual parts


    switch (msg->type) {
        case AUTH:
            content_length = find_null_character_position(buffer + offset, USERNAME_MAX_LENGTH) + 1;
            memcpy(msg->data.auth.username, buffer + offset, content_length);
            offset += content_length;
            content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.auth.displayName, buffer + offset, content_length);
            offset += content_length;
            content_length = find_null_character_position(buffer + offset, SECRET_MAX_LENGHT) + 1;
            memcpy(msg->data.auth.secret, buffer + offset, content_length);
            offset += content_length; 
            break;
        case JOIN:
            content_length = find_null_character_position(buffer + offset, CHANNEL_ID_MAX_LENGTH) + 1;
            memcpy(msg->data.join.channelID, buffer + offset, content_length);
            offset += content_length;
            content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.join.displayName, buffer + offset, content_length);
            break;
        case ERR:
            content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.err.displayName, buffer + offset, content_length);
            offset += content_length;
            content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.err.messageContent, buffer + offset, content_length);
            break;
        case BYE:
            break;
        case MSG:
            content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.msg.displayName, buffer + offset, content_length);
            offset += content_length;
            content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.msg.messageContent, buffer + offset, content_length);
            break;
        case REPLY:
            memcpy(&(msg->data.reply.result),buffer+offset, sizeof(uint8_t));  // Result
            offset += sizeof(uint8_t);

            memcpy(&(msg->data.reply.ref_messageID), buffer+offset, sizeof(uint16_t));  // Ref_MessageID 
            offset += sizeof(uint16_t);

            content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.reply.messageContent, buffer + offset, content_length);
            break;
        default:
            // Change this for whole UDP logic implementation
            fprintf(stderr, "Not supported message type from server\n");
            exit(1);
    }
}


// Main function for the udp communication
void udp_main(struct sockaddr_in servaddr, int server_port, int udp_main, int max_retransmissions){

    // Message buffers
    char input_line[FULL_MESSAGE_BUFFER + 1];
    char line_to_send_from_client[FULL_MESSAGE_BUFFER + 1];
    char received_message[FULL_MESSAGE_BUFFER + 1];

    // Variable that stores states in finite state machine
    State current_state = START_STATE;

    // Socket creating
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int client_socket = socket(family, type, 0);
    if (client_socket <= 0) 
        {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
        }
    
    // Bind netcat server on me
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

    // Message exchange loop
    while(true){ 

        // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout
        if (ret == -1) {
            perror("ERROR: poll");
            exit(EXIT_FAILURE);
        }

        // Incoming message from stdin(user)
        if (fds[0].revents & POLLIN) {

            // Beginnings of user commands
            char *AUTH_COMMAND_TO_CHECK = "/auth";
            char *JOIN_COMMAND_TO_CHECK = "/join";
            char *RENAME_COMMAND_TO_CHECK = "/rename";
            char *HELP_COMMAND_TO_CHECK = "/help";

            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

            // Authentification in the start state without a confirmation message
            if (current_state == START_STATE){

                // Checks beginning of the inputed command from user
                if (strncmp(input_line, AUTH_COMMAND_TO_CHECK, strlen(AUTH_COMMAND_TO_CHECK)) == 0) {
                    uint8_t type = 0x02;
                    char delete_auth_zaciatok[5] = "";
                    char username[USERNAME_MAX_LENGTH + 1] = "";
                    char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";
                    char secret[SECRET_MAX_LENGHT + 1] = "";

                    // Splits command into subparts
                    sscanf(input_line, "%s %s %s %s", delete_auth_zaciatok, username, secret, display_name);

                    size_t username_length = strlen(username);
                    size_t display_name_length = strlen(display_name);
                    size_t secret_length = strlen(secret);

                    // 3 is number of terminated zeros, never ever again write code like this
                    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + username_length + display_name_length + secret_length + 3;  

                    // Build up buffer to send correct message to a server
                    uint8_t *ptr = (uint8_t *)line_to_send_from_client;
                    *ptr++ = type;
                    memcpy(ptr, &msg_id, sizeof(uint16_t));
                    ptr += sizeof(uint16_t);
                    strcpy((char *)ptr, username);
                    ptr += username_length + 1;
                    strcpy((char *)ptr, display_name);
                    ptr += display_name_length + 1;
                    strcpy((char *)ptr, secret);

                    // Send a built message to the server
                    if (sendto(client_socket, line_to_send_from_client, message_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                    perror("Error: sendto failed");
                    close(client_socket);
                    exit(EXIT_FAILURE);
                    }

                    msg_id++;
                    current_state = OPEN_STATE;
                } else {
                    current_state = ERROR_STATE;
                }
            }

            // Change this for whole UDP logic implementation
            if(current_state == ERROR_STATE){
                printf("I am in error state \n");
                printf("To do : logic of error state \n");
            }

            // Logic for the rest of the user commands
            if (strncmp(input_line, JOIN_COMMAND_TO_CHECK, strlen(JOIN_COMMAND_TO_CHECK)) == 0) {
                uint8_t type = 0x03;
                char delete_join_zaciatok[5] = "";
                char channelID[CHANNEL_ID_MAX_LENGTH + 1] = "";
                char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";

                sscanf(input_line, "%s %s %s", delete_join_zaciatok, channelID, display_name);

                size_t channelID_length = strlen(channelID);
                size_t display_name_length = strlen(display_name);

                size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + channelID_length + display_name_length + 2;  

                // Populate buffer
                uint8_t *ptr = (uint8_t *)line_to_send_from_client;
                *ptr++ = type;
                memcpy(ptr, &msg_id, sizeof(uint16_t));
                ptr += sizeof(uint16_t);
                strcpy((char *)ptr, channelID);
                ptr += channelID_length + 1;
                strcpy((char *)ptr, display_name);

                if (sendto(client_socket, line_to_send_from_client, message_size, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
                perror("Error: sendto failed");
                close(client_socket);
                exit(EXIT_FAILURE);
                }

                msg_id++;
            } else if (strncmp(input_line, RENAME_COMMAND_TO_CHECK, strlen(RENAME_COMMAND_TO_CHECK)) == 0) {
                puts("Locally changes the display name of the user to be sent with new messages/selected commands");
            } else if (strncmp(input_line, HELP_COMMAND_TO_CHECK, strlen(HELP_COMMAND_TO_CHECK)) == 0) {
                printf("Commands:\n");
                printf("  /auth\n");
                printf("  PARAMETERS: {Username} {Secret} {DisplayName}\n");
                printf("  DESCRIPTION: add Description");
                printf("  ...");
            } 
        }

        // Receiving one example of a message, In this version only for netcat testing after authentification
        if (current_state == OPEN_STATE){
            recvfrom(client_socket, &received_message, 1501, 0, (struct sockaddr *)&servaddr, sizeof(servaddr) < 0);

        }
    }
}