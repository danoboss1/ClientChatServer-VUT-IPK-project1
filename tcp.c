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
#include <signal.h>

#include "tcp.h"


// Stores display_name a for further usage
char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";

// Poll file desriptors structure
struct pollfd fds[2];

// CTRL+C program termination signal
#define SIGINT 2


// Checks server messages and assigns the correct message type
MessageType Handle_server_messages_tcp(char *received_message){

    MessageType checked_msg;
    
    // Variables for printing server messages in correct format
    char message_content_server_tcp[1401];
    char display_name_server_tcp[21];

    // Regexes for server message formats
    regex_t msg_regex, err_regex, reply_ok_regex, reply_nok_regex, bye_regex;
    const char *msg_pattern = "^MSG FROM ([!-~]{1,20}) IS (.{1,1400})$";
    const char *err_pattern = "^ERR FROM ([!-~]{1,20}) IS (.{1,1400})$";
    const char *reply_ok_pattern = "^REPLY OK IS (.{1,1400})$";
    const char *reply_nok_pattern = "^REPLY NOK IS (.{1,1400})$";
    const char *bye_pattern = "^BYE";

    // Compile regular expressions
    if (regcomp(&msg_regex, msg_pattern, REG_EXTENDED) != 0 ||
        regcomp(&err_regex, err_pattern, REG_EXTENDED) != 0 ||
        regcomp(&reply_ok_regex, reply_ok_pattern, REG_EXTENDED) != 0 ||
        regcomp(&reply_nok_regex, reply_nok_pattern, REG_EXTENDED) != 0 ||
        regcomp(&bye_regex, bye_pattern, REG_EXTENDED) != 0) {
        fprintf(stderr, "Failed to compile regex pattern\n");
        exit(EXIT_FAILURE);
    }

    // Match received message against each regex
    if (regexec(&msg_regex, received_message, 0, NULL, 0) == 0) {
        // Received message matches the MSG regex
        checked_msg = MESSAGE_TYPE_MSG;
    } else if (regexec(&err_regex, received_message, 0, NULL, 0) == 0) {
        // Received message matches the ERR regex
        checked_msg = MESSAGE_TYPE_ERR;
    } else if (regexec(&reply_ok_regex, received_message, 0, NULL, 0) == 0) {
        // Received message matches the REPLY regex
        checked_msg = MESSAGE_TYPE_REPLY;
    } else if (regexec(&reply_nok_regex, received_message, 0, NULL, 0) == 0) {
        // Received message matches the REPLY regex
        checked_msg = MESSAGE_TYPE_NOT_REPLY;
    } else if (regexec(&bye_regex, received_message, 0, NULL, 0) == 0) {
        // Received message matches the BYE regex
        checked_msg = MESSAGE_TYPE_BYE;
    } else {
        // Unsupported message from server
        checked_msg = MESSAGE_TYPE_WRONG;
    }

    // Print server messages based on message type
    switch (checked_msg) {
        case MESSAGE_TYPE_MSG:
            {
                sscanf(received_message, "MSG FROM %20[^ ] IS %1400[^\n]", display_name_server_tcp, message_content_server_tcp);
                printf("%s: %s\n", display_name_server_tcp, message_content_server_tcp);
            }
            break;
        case MESSAGE_TYPE_ERR:
            {
                sscanf(received_message, "ERR FROM %20[^ ] IS %1400[^\n]", display_name_server_tcp, message_content_server_tcp);
                fprintf(stderr, "ERR FROM %s: %s\n", display_name_server_tcp, message_content_server_tcp);
            }
            break;
        case MESSAGE_TYPE_REPLY:
            {
                sscanf(received_message, "REPLY OK IS %1401[^\n]", message_content_server_tcp);
                fprintf(stderr, "Success: %s\n", message_content_server_tcp);
            }
            break;
        case MESSAGE_TYPE_NOT_REPLY:
            {
                sscanf(received_message, "REPLY NOK IS %1401[^\n]", message_content_server_tcp);
                fprintf(stderr, "Failure: %s\n", message_content_server_tcp);
            }
            break;
        default:
            // For other types, no printout is required
            break;
    }

    // Cleanup: Free resources
    regfree(&msg_regex);
    regfree(&err_regex);
    regfree(&reply_ok_regex);
    regfree(&reply_nok_regex);
    regfree(&bye_regex);

    return checked_msg;
}

// Signal handeling function, correct termination
void tcp_handler (int signum){
    write(fds[1].fd, "BYE\r\n", 5);
    exit(0);
}

// Sends BYE message from user to a server
void user_BYE(char *line_to_send_from_client, int sockfd){
        size_t message_size;
        ssize_t bytes_sent;

        sprintf(line_to_send_from_client, "BYE\r\n");

        message_size  = strlen(line_to_send_from_client);

        bytes_sent = write(sockfd, line_to_send_from_client, message_size);
        if (bytes_sent < 0) {
            perror("ERROR: sending message");
            exit(EXIT_FAILURE);
        }
}

// Sends ERROR message from user to a server
void ERROR_from_user_to_server(char *line_to_send_from_client, int sockfd){
        size_t message_size;
        ssize_t bytes_sent;

        //ERR FROM {DisplayName} IS {MessageContent}\r\n
        sprintf(line_to_send_from_client, "ERR FROM %s IS Unexpected message in this state\r\n", display_name);

        message_size  = strlen(line_to_send_from_client);

        bytes_sent = write(sockfd, line_to_send_from_client, message_size);
        if (bytes_sent < 0) {
            perror("ERROR: sending message");
            exit(EXIT_FAILURE);
        }
}

// Processes user commands and messages
MessageType Handle_user_input_tcp(char *input_line, char *line_to_send_from_client, int sockfd){

    MessageType checked_msg;

    size_t message_size;  // size of buffer to be send to the server
    ssize_t bytes_sent;
    bool going_to_send_message = false;

    // Beginnings of user commands
    char *AUTH_COMMAND_TO_CHECK = "/auth";
    char *JOIN_COMMAND_TO_CHECK = "/join";
    char *RENAME_COMMAND_TO_CHECK = "/rename";
    char *HELP_COMMAND_TO_CHECK = "/help";

    // Auth command
    if (strncmp(input_line, AUTH_COMMAND_TO_CHECK, strlen(AUTH_COMMAND_TO_CHECK)) == 0) {
        char delete_auth_zaciatok[5] = "";
        char username[USERNAME_MAX_LENGTH + 1] = "";
        char secret[SECRET_MAX_LENGHT + 1] = "";

        // Splits command to a required logical parts
        sscanf(input_line, "%s %s %s %s", delete_auth_zaciatok, username, secret, display_name);

        // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
        sprintf(line_to_send_from_client, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);

        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_AUTH;
    // Join command
    } else if (strncmp(input_line, JOIN_COMMAND_TO_CHECK, strlen(JOIN_COMMAND_TO_CHECK)) == 0) {
        char delete_join_zaciatok[5] = "";
        char channelID[CHANNEL_ID_MAX_LENGTH + 1] = "";

        // Splits command to a required logical parts
        sscanf(input_line, "%s %s", delete_join_zaciatok, channelID);

        // JOIN {ChannelID} AS {DisplayName}\r\n
        sprintf(line_to_send_from_client, "JOIN %s AS %s\r\n", channelID, display_name);
        
        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_JOIN;
    // Rename command
    } else if (strncmp(input_line, RENAME_COMMAND_TO_CHECK, strlen(RENAME_COMMAND_TO_CHECK)) == 0) {
        char delete_rename_zaciatok[7] = "";

        // Splits command to a required logical parts
        sscanf(input_line, "%s %s", delete_rename_zaciatok, display_name);

        checked_msg = MESSAGE_TYPE_NONE;
    // Help command
    } else if (strncmp(input_line, HELP_COMMAND_TO_CHECK, strlen(HELP_COMMAND_TO_CHECK)) == 0) {
        printf("U have used /help command!");
        printf("Commands:\n");
        printf("  /auth\n");
        printf("  PARAMETERS: {Username} {Secret} {DisplayName}\n");
        printf("  DESCRIPTION: Sends AUTH message with the data provided from the command to the server\n");
        printf("  /join\n");
        printf("  PARAMETERS: {channelID}\n");
        printf("  DESCRIPTION: Sends JOIN message with channel name from the command to the server\n");
        printf("  /rename\n");
        printf("  PARAMETERS: {DisplayName}\n");
        printf("  DESCRIPTION: Locally changes the display name of the user to be sent with new messages/selected commands\n");
        printf("  /help\n");
        printf("  PARAMETERS: None\n");
        printf("  DESCRIPTION: Prints out supported local commands with their parameters and a description\n");

        checked_msg = MESSAGE_TYPE_NONE;
    // Message
    } else {
        size_t input_line_length = strlen(input_line);
        input_line[input_line_length - 1] = '\0';

        //MSG FROM {DisplayName} IS {MessageContent}\r\n
        sprintf(line_to_send_from_client, "MSG FROM %s IS %s\r\n", display_name, input_line);

        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_MSG;
    }

    if (going_to_send_message){
        message_size  = strlen(line_to_send_from_client);

        // Sending message to the server
        bytes_sent = write(sockfd, line_to_send_from_client, message_size);
        if (bytes_sent < 0) {
            perror("ERROR: sending message");
            exit(EXIT_FAILURE);
        }
    }

    return checked_msg;
}

// Main function for the tcp communication
void tcp_main(struct sockaddr_in servaddr, int server_port) {

    // Message buffers
    char input_line[FULL_MESSAGE_BUFFER + 1];
    char line_to_send_from_client[FULL_MESSAGE_BUFFER + 1];
    char received_message[FULL_MESSAGE_BUFFER + 1];

    // Variable that stores states in finite state machine
    State current_state = START_STATE;

    MessageType msg_type;

    // Socket creating
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR: opening socket");
        exit(EXIT_FAILURE);
    }

    // Connecting to the server
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0) {
        perror("ERROR: connecting");
        exit(EXIT_FAILURE);
    }

    // Define pollfd structures
    fds[0].fd = STDIN_FILENO; // For stdin
    fds[0].events = POLLIN;
    fds[1].fd = sockfd; // For socket
    fds[1].events = POLLIN;

    // Message exchange loop
    while(true){ 

        // Checks if u want to terminate program through CTRL+C
        signal(SIGINT, tcp_handler);

        // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout
        if (ret == -1) {
            perror("ERROR: poll");
            exit(EXIT_FAILURE);
        }

        // Incoming message from stdin(user)
        if (fds[0].revents & POLLIN) {
            
            // Load user message
            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

            // Process user message and returns a message type
            msg_type = Handle_user_input_tcp(input_line, line_to_send_from_client, sockfd);

            // Finite state machine for processing user inputs
            if (current_state == START_STATE){
                if (msg_type == MESSAGE_TYPE_AUTH) {
                    current_state = AUTH_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    fprintf(stderr, "ERR: Cannot send message in this state, use /help command\n");
                    current_state = START_STATE;
                } else if (msg_type == MESSAGE_TYPE_JOIN){
                    fprintf(stderr, "ERR: Cannot join server by channelID in this state, use /help command\n");
                    current_state = START_STATE;
                } else if (msg_type == MESSAGE_TYPE_NONE) {
                    current_state = START_STATE;
                } else {
                    fprintf(stderr, "ERR: Not supported command or message, use /help command\n");
                    current_state = START_STATE;
                }
            } else if (current_state == AUTH_STATE){
                if (msg_type == MESSAGE_TYPE_BYE) {
                    user_BYE(line_to_send_from_client, sockfd);
                    current_state = END_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    fprintf(stderr, "ERR: Cannot send message in this state, use /help command\n");
                    current_state = AUTH_STATE;
                } else if (msg_type == MESSAGE_TYPE_JOIN){
                    fprintf(stderr, "ERR: Cannot join server by channelID in this state, use /help command\n");
                    current_state = AUTH_STATE;
                } else if (msg_type == MESSAGE_TYPE_NONE) {
                    current_state = AUTH_STATE;
                } else {
                    fprintf(stderr, "ERR: Not supported command or message, use /help command\n");
                    current_state = AUTH_STATE; 
                }
            } else if (current_state == OPEN_STATE){
                if (msg_type == MESSAGE_TYPE_JOIN) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_NONE) {
                    current_state = OPEN_STATE;
                } else {
                    fprintf(stderr, "ERR: Not supported command or message, use /help command\n");
                    current_state = OPEN_STATE; 
                }
            }
        }
        
        // Incoming message from server
        if (fds[1].revents & POLLIN) {
            ssize_t bytes_received;

            // Read server message
            bytes_received = read(sockfd, received_message, sizeof(received_message));
            if (bytes_received < 0) {
                perror("ERROR: receiving message");
                exit(EXIT_FAILURE);
            }

            // Adjust the length of the received string, removing /r/n from the buffer
            if (bytes_received >= 2) {
                received_message[bytes_received - 2] = '\0'; // Move the terminating null character 2 bytes to the left
            } else {
                // Handle the case where there are less than 2 bytes received
                memset(received_message, '\0', sizeof(received_message));
            }

            // Process server message and returns a message type
            msg_type = Handle_server_messages_tcp(received_message);

            // Finite state machine for processing server messages
            if (current_state == AUTH_STATE){
                if (msg_type == MESSAGE_TYPE_NOT_REPLY) {
                    current_state = AUTH_STATE;
                } else if (msg_type == MESSAGE_TYPE_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_ERR) {
                    user_BYE(line_to_send_from_client, sockfd);
                    current_state = END_STATE;
                } else {
                    ERROR_from_user_to_server(line_to_send_from_client, sockfd);
                    current_state = ERROR_STATE;
                    user_BYE(line_to_send_from_client, sockfd);
                    current_state = END_STATE;
                }
            } else if (current_state == OPEN_STATE) {
                if (msg_type == MESSAGE_TYPE_NOT_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_REPLY) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_ERR) {
                    user_BYE(line_to_send_from_client, sockfd);
                    current_state = END_STATE;
                } else if (msg_type == MESSAGE_TYPE_BYE) {
                    current_state = END_STATE;
                } else {
                    ERROR_from_user_to_server(line_to_send_from_client, sockfd);
                    current_state = ERROR_STATE;
                    user_BYE(line_to_send_from_client, sockfd);
                    current_state = END_STATE;
                }
            } else if (current_state == END_STATE)  {
                current_state = END_STATE;
            }

        }

        // Terminating communication
        if (current_state == END_STATE)  {
            break;
        }
    }

    // Closing socket
    close(sockfd);
    exit(0);
}