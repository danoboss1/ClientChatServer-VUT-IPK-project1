#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "udp.h"

#include <assert.h>

// TU JE NIECO ZLE S TYMI CISLAMI A ICH POSIELANIM
void test_CONFIRM(){
    uint8_t type = 0x00;
    uint16_t ref_message_id = 123;

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t);  // Calculate the size of the message in the buffer

    char buffer[message_size];

    buffer[0] = type;
    memcpy(buffer + 1, &ref_message_id, sizeof(uint16_t));

    Message received_message;

    // Parse the received message buffer using Handle_message_from_server
    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
}

void test_REPLY() {
    uint8_t type = 0x01;
    uint16_t message_id = 200;
    uint8_t result = 1; // Assuming success
    uint16_t ref_message_id_reply = 456;
    char message_contents[] = "Sample message contents"; // Example message contents
    size_t message_contents_length = strlen(message_contents);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + message_contents_length + 1;  

    char buffer[message_size];

    // Populate buffer
    buffer[0] = type;
    memcpy(buffer + 1, &message_id, sizeof(uint16_t));
    buffer[3] = result;
    memcpy(buffer + 4, &ref_message_id_reply, sizeof(uint16_t));
    memcpy(buffer + 6, message_contents, message_contents_length);
    buffer[6 + message_contents_length] = '\0'; // Null-terminate message contents

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("Result: %d\n", received_message.data.reply.result);
    printf("Ref Message ID: %d\n", received_message.data.reply.ref_messageID);
    printf("Message Contents: %s\n", received_message.data.reply.messageContent);
}

// Implement tests for other message types similarly
void test_AUTH() {
    uint8_t type = 0x02;
    uint16_t message_id = 200;

    char username_test[] = "robko";
    char display_name_test[] = "xrobert04";
    char secret_test[] = "nema";

    // size_t message_contents_length = strlen(message_contents);

    // Populate buffer
    size_t offset = sizeof(uint8_t) + sizeof(uint16_t);

    size_t username_length = strlen(username_test);
    size_t display_name_length = strlen(display_name_test);
    // printf("%ld", display_name_length);
    size_t secret_length = strlen(secret_test);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + username_length + display_name_length + secret_length + 1;  

    char buffer[message_size];

    // Populate buffer
    uint8_t *ptr = (uint8_t *)buffer;
    *ptr++ = type;
    memcpy(ptr, &message_id, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    strcpy(ptr, username_test);
    ptr += username_length + 1;
    strcpy(ptr, display_name_test);
    ptr += display_name_length + 1;
    strcpy(ptr, secret_test);

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("auth username: %s\n", received_message.data.auth.username);
    printf("auth display_name: %s\n", received_message.data.auth.displayName);
    printf("auth secret: %s\n", received_message.data.auth.secret);
}

// Implement tests for other message types similarly
void test_JOIN() {
    uint8_t type = 0x03;
    uint16_t message_id = 300;

    char channelID[] = "channelID_300";
    char display_name_test[] = "xSERVER300";

    // Populate buffer
    size_t offset = sizeof(uint8_t) + sizeof(uint16_t);

    size_t channelID_length = strlen(channelID);
    size_t display_name_length = strlen(display_name_test);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + channelID_length + display_name_length + 1;  

    char buffer[message_size];

    // Populate buffer
    uint8_t *ptr = (uint8_t *)buffer;
    *ptr++ = type;
    memcpy(ptr, &message_id, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    strcpy(ptr, channelID);
    ptr += channelID_length + 1;
    strcpy(ptr, display_name_test);

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("join channelID: %s\n", received_message.data.join.channelID);
    printf("join display_name: %s\n", received_message.data.join.displayName);
}

void test_MSG() {
    uint8_t type = 0x04;
    uint16_t message_id = 400;

    char display_name_test[] = "messenger400";
    char message_contents_test[] = "Sample message contents";


    // Populate buffer
    size_t offset = sizeof(uint8_t) + sizeof(uint16_t);

    size_t display_name_length = strlen(display_name_test);
    size_t message_contents_length = strlen(message_contents_test);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + display_name_length + message_contents_length + 1;  

    char buffer[message_size];

    // Populate buffer
    uint8_t *ptr = (uint8_t *)buffer;
    *ptr++ = type;
    memcpy(ptr, &message_id, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    strcpy(ptr, display_name_test);
    ptr += display_name_length + 1;
    strcpy(ptr, message_contents_test);

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("msg display_name: %s\n", received_message.data.msg.displayName);
    printf("msg messagecontents: %s\n", received_message.data.msg.messageContent);
}


void test_ERR() {
    uint8_t type = 0xFE;
    uint16_t message_id = 800;

    char display_name_test[] = "messenger800";
    char message_contents_test[] = "Sample message contents";


    // Populate buffer
    size_t offset = sizeof(uint8_t) + sizeof(uint16_t);

    size_t display_name_length = strlen(display_name_test);
    size_t message_contents_length = strlen(message_contents_test);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + display_name_length + message_contents_length + 1;  

    char buffer[message_size];

    // Populate buffer
    uint8_t *ptr = (uint8_t *)buffer;
    *ptr++ = type;
    memcpy(ptr, &message_id, sizeof(uint16_t));
    ptr += sizeof(uint16_t);
    strcpy(ptr, display_name_test);
    ptr += display_name_length + 1;
    strcpy(ptr, message_contents_test);

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("err display_name: %s\n", received_message.data.err.displayName);
    printf("err messagecontents: %s\n", received_message.data.err.messageContent);
}

void test_BYE() {
    uint8_t type = 0xFF;
    uint16_t message_id = 800;

    // Populate buffer
    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t);

    char buffer[message_size];

    // Populate buffer
    uint8_t *ptr = (uint8_t *)buffer;
    *ptr++ = type;
    memcpy(ptr, &message_id, sizeof(uint16_t));

    Message received_message;

    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);
    printf("BYE FUNGUJE DOBRE \n");
}

int main() {
    // Create a Message structure for the confirm message
    test_CONFIRM();
    test_REPLY();
    test_AUTH();
    test_JOIN();
    test_MSG();
    test_ERR();
    test_BYE();

    return 0;
}