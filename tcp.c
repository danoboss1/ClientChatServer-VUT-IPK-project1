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

char display_name[DISPLAY_NAME_MAX_LENGTH + 1] = "";

struct pollfd fds[2];

#define SIGINT 2


MessageType Handle_server_messages_tcp(char *received_message){
    MessageType checked_msg;
    // prechadzam vsetky regexy
    // ak najdem zhodu vypisem spravu a vratim typ message
    char message_content_server_tcp[1401];
    char display_name_server_tcp[21];


    // ak nie vypisem internal error -> idem do error state-u
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

    // Regular expressions are compiled, you can now use them
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
        // Received message does not match any of the regexes
        // tu bude error od uzivatela
        printf("Received message does not match any known pattern\n");
    }

    // tuto nejako to vypisovanie dorobit
        // Print messages based on message type
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

// signal handeling function, wirites to designated FD
void tcp_handler (int signum){
    write(fds[1].fd, "BYE\r\n", 5);
    exit(0);
}

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

void ERROR_from_user_to_server(char *line_to_send_from_client, int sockfd){
        size_t message_size;
        ssize_t bytes_sent;

        sprintf(line_to_send_from_client, "ERR FROM %s IS Unexpected message in this state\r\n", display_name);

        message_size  = strlen(line_to_send_from_client);

        bytes_sent = write(sockfd, line_to_send_from_client, message_size);
        if (bytes_sent < 0) {
            perror("ERROR: sending message");
            exit(EXIT_FAILURE);
        }

        //ERR FROM {DisplayName} IS {MessageContent}\r\n
        //sprintf(line_to_send_from_client, "JOIN %s AS %s\r\n", channelID, display_name);
}

MessageType Handle_user_input_tcp(char *input_line, char *line_to_send_from_client, int sockfd){
    MessageType checked_msg;

    size_t message_size;
    ssize_t bytes_sent;
    bool going_to_send_message = false;

    char *AUTH_COMMAND_TO_CHECK = "/auth";
    char *JOIN_COMMAND_TO_CHECK = "/join";
    char *RENAME_COMMAND_TO_CHECK = "/rename";
    char *HELP_COMMAND_TO_CHECK = "/help";

    // AUTH PREHADZUJEM PRIDE MENO,SECRET,DNAME A POSIELAM MENO DNAME SECRET
    if (strncmp(input_line, AUTH_COMMAND_TO_CHECK, strlen(AUTH_COMMAND_TO_CHECK)) == 0) {
        char delete_auth_zaciatok[5] = "";
        char username[USERNAME_MAX_LENGTH + 1] = "";
        char secret[SECRET_MAX_LENGHT + 1] = "";

        // tato FUNKCIA MI TO ROZDELI DO PREMENNYCH
        sscanf(input_line, "%s %s %s %s", delete_auth_zaciatok, username, secret, display_name);

        sprintf(line_to_send_from_client, "AUTH %s AS %s USING %s\r\n", username, display_name, secret);

        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_AUTH;
        // AUTH {Username} AS {DisplayName} USING {Secret}\r\n
    } else if (strncmp(input_line, JOIN_COMMAND_TO_CHECK, strlen(JOIN_COMMAND_TO_CHECK)) == 0) {
        char delete_join_zaciatok[5] = "";
        char channelID[CHANNEL_ID_MAX_LENGTH + 1] = "";

        sscanf(input_line, "%s %s", delete_join_zaciatok, channelID);

        sprintf(line_to_send_from_client, "JOIN %s AS %s\r\n", channelID, display_name);
        
        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_JOIN;
        // JOIN {ChannelID} AS {DisplayName}\r\n
    } else if (strncmp(input_line, RENAME_COMMAND_TO_CHECK, strlen(RENAME_COMMAND_TO_CHECK)) == 0) {
        char delete_rename_zaciatok[7] = "";

        sscanf(input_line, "%s %s", delete_rename_zaciatok, display_name);

        checked_msg = MESSAGE_TYPE_NONE;
        // tu menim iba display name a nic neposielam
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
    } else {
        sprintf(line_to_send_from_client, "MSG FROM %s IS %s\r\n", display_name, input_line);

        going_to_send_message = true;
        checked_msg = MESSAGE_TYPE_MSG;
        //MSG FROM {DisplayName} IS {MessageContent}\r\n
    }

    if (going_to_send_message){
        message_size  = strlen(line_to_send_from_client);


        // Use write() function to send data
        bytes_sent = write(sockfd, line_to_send_from_client, message_size);
        if (bytes_sent < 0) {
            perror("ERROR: sending message");
            exit(EXIT_FAILURE);
        }
        // write a read
        // vlozim do prazdneho bufferu spravu a zistim aky velky je ten buffer

        return checked_msg;
    }
}


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
    fds[0].fd = STDIN_FILENO; // For stdin
    fds[0].events = POLLIN;
    fds[1].fd = sockfd; // For socket
    fds[1].events = POLLIN;

    while(true){ 
        // Call tcp_handler manually
        // tcp_handler(SIGINT); // This will call tcp_handler as if SIGINT was receive
    signal(SIGINT, tcp_handler);

        // tu bude nejaky stavovy podla zadania
        // do terminalu pisem rovnako, len serveru posielam iny format, aj on mne posiela v inom formate

        // // Wait for events on multiple file descriptors
        int ret = poll(fds, 2, -1); // -1 for indefinite timeout

        // tuto budem este informovat uzivatela ked zada zly vstup
        if (fds[0].revents & POLLIN) {
            fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);

            msg_type = Handle_user_input_tcp(input_line, line_to_send_from_client, sockfd);

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
                } 
                // tu bude mozno nejaky error pri inych typoch
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
                }
                // tu bude mozno nejaky error pri inych typoch
            } else if (current_state == OPEN_STATE){
                if (msg_type == MESSAGE_TYPE_JOIN) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_MSG) {
                    current_state = OPEN_STATE;
                } else if (msg_type == MESSAGE_TYPE_NONE) {
                    current_state = OPEN_STATE;
                }
            }

                // write a read
                // vlozim do prazdneho bufferu spravu a zistim aky velky je ten buffer


                // AUTH {Username} AS {DisplayName} USING {Secret}\r\n

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

            // Adjust the length of the received string
            if (bytes_received >= 2) {
                received_message[bytes_received - 2] = '\0'; // Move the terminating null character 2 bytes to the left
            } else {
                // Handle the case where there are less than 2 bytes received
                // For example, you may choose to set the entire received_message to '\0'
                memset(received_message, '\0', sizeof(received_message));
            }

            // tu spracujem spravu a zistim aky je to typ
            // message_type = 'ERR';

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


        if (current_state == END_STATE)  {
            break;
        }
    }

    close(sockfd);
    exit(0);

}