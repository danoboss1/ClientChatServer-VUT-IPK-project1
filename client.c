// to dalsieho suboru si mozem definovat vsetky struktury a tak
// definujes struktury na vsetky rozne typy
// dostanes spravy type a id vies aku ma velkost
// toto vsetko v 1500 dlhom buffery
// a potom si este definujes max_lenghy 
// podla max_lengu jednotlivych typov v contente posielas do fukncie, ktora ti zisti dlzku retazca daneho typu, toto cez ukazatele
// a potom sa posunies o konkretnu dlzku v skutocnom kontexte

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DISPLAY_NAME_MAX_LENGTH 20
#define CHANNEL_ID_MAX_LENGTH 20
#define USERNAME_MAX_LENGTH 20  
#define SECRET_MAX_LENGHT 128
#define MESSAGE_CONTENT_MAX_LENGTH 1400

#define FULL_MESSAGE_BUFFER 1500


// Define message types
typedef enum {
    MESSAGE_TYPE_AUTH,
    MESSAGE_TYPE_JOIN,
    MESSAGE_TYPE_ERR,
    MESSAGE_TYPE_BYE,
    MESSAGE_TYPE_MSG,
    MESSAGE_TYPE_REPLY,
    MESSAGE_TYPE_NOT_REPLY
} MessageType;

// Define structures for message arguments
typedef struct {
    char username[USERNAME_MAX_LENGTH + 1]; 
    char displayName[DISPLAY_NAME_MAX_LENGTH + 1]; 
    char secret[SECRET_MAX_LENGHT + 1]; 
} StructAuth;

typedef struct {
    char channelID[CHANNEL_ID_MAX_LENGTH + 1]; 
    char displayName[DISPLAY_NAME_MAX_LENGTH + 1]; 
} StructJoin;

typedef struct {
    char displayName[DISPLAY_NAME_MAX_LENGTH + 1]; 
    char messageContent[MESSAGE_CONTENT_MAX_LENGTH + 1]; 
} StructErr, StructMsg;

typedef struct {
    uint8_t result;
    uint16_t ref_messageID;
    char messageContent[MESSAGE_CONTENT_MAX_LENGTH + 1]; 
} StructReply;

// Define message structure
typedef struct {
    MessageType type;
    uint16_t messageID;
    union {
        StructAuth auth;
        StructJoin join;
        StructErr err;
        StructMsg msg;
        StructReply reply;
    } data;
} Message;

size_t find_null_character_position(const char *str, size_t max_length) {
    size_t length = strnlen(str, max_length); // Určí dĺžku reťazca, ale najviac max_length znakov
    return length;
}

// vycistit si kod od nepotrebneho bordelu
// zistit co vsetko ako funguje a okomentovat si to 
// loop na spravy
// parameters handle
// rozdelenie spravy na tie jednotlive veci
// specifikacie vymienania sprav
// logika stavoveho automatu

#define PORT 4567
#define MAXLINE 100

// Define message types
#define CONFIRM 0x00
#define REPLY   0x01
#define AUTH    0x02
#define JOIN    0x03
#define MSG     0x04
#define ERR     0xFE
#define BYE     0xFF


// Define message structure
// struct Message {
//     uint8_t type;
//     uint16_t messageID;
//     char content[1400];
// };

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

// Function to receive message
Message receiveMessage(int sockfd, struct sockaddr_in* server_addr) {
    Message msg;
    socklen_t len = sizeof(*server_addr);
    // Receive message
    recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr*)server_addr, &len);
    return msg;
}

//FUNKCIA NA HANDLOVANIE SPRAVY
size_t Handle_message_from_server(const char *buffer, Message *msg){
    memcpy(&(msg->type),buffer, sizeof(uint8_t));
    memcpy(&(msg->messageID), buffer+1, sizeof(uint16_t));

    // size_t je 64 bitovy unsigned integer
    size_t offset = sizeof(uint8_t) + sizeof(uint16_t); // Offset for the content

    switch (msg->type) {
        case AUTH:
            size_t content_length = find_null_character_position(buffer + offset, USERNAME_MAX_LENGTH) + 1;
            memcpy(msg->data.auth.username, buffer + offset, content_length);
            offset += content_length;
            size_t content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.auth.displayName, buffer + offset, content_length);
            offset += content_length;
            size_t content_length = find_null_character_position(buffer + offset, SECRET_MAX_LENGHT) + 1;
            memcpy(msg->data.auth.secret, buffer + offset, content_length);
            offset += content_length; // to idk ci potrebujem
        case JOIN:
            size_t content_length = find_null_character_position(buffer + offset, CHANNEL_ID_MAX_LENGTH) + 1;
            memcpy(msg->data.join.channelID, buffer + offset, content_length);
            offset += content_length;
            size_t content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.join.displayName, buffer + offset, content_length);
        case ERR:
            size_t content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.err.displayName, buffer + offset, content_length);
            offset += content_length;
            size_t content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.err.messageContent, buffer + offset, content_length);
        case BYE:
            break;
        case MSG:
            size_t content_length = find_null_character_position(buffer + offset, DISPLAY_NAME_MAX_LENGTH) + 1;
            memcpy(msg->data.msg.displayName, buffer + offset, content_length);
            offset += content_length;
            size_t content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.msg.messageContent, buffer + offset, content_length);
        case REPLY:
            // REPLY JE DEFINOVANY NA 3 STAVY V ZADANI
            memcpy(&(msg->data.reply.result),buffer, sizeof(uint8_t));
            memcpy(&(msg->data.reply.ref_messageID), buffer+1, sizeof(uint16_t));

            offset += sizeof(uint8_t);
            offset += sizeof(uint16_t);

            size_t content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.reply.messageContent, buffer + offset, content_length);
        default:
            break;
            //TU MOZNO NEJAKA CHYBA
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
    fgets(input_line, FULL_MESSAGE_BUFFER + 1, stdin);
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

    // puts("Neposlal som nikde nic");
    while(true){ 

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