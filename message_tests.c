#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "client.h"

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
    uint16_t ref_message_id = 123;
    uint8_t result = 1; // Assuming success
    uint16_t ref_message_id_reply = 456;
    char message_contents[] = "Sample message contents"; // Example message contents
    size_t message_contents_length = strlen(message_contents);

    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + message_contents_length + 1;  

    char buffer[message_size];

    // Populate buffer
    buffer[0] = type;
    memcpy(buffer + 1, &ref_message_id, sizeof(uint16_t));
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

int main() {
    // Create a Message structure for the confirm message
    test_CONFIRM();
    // test_REPLY();
    


    return 0;
}

// // Write your test cases here...
// void test_Handle_message_from_server() {
//     // Define your test cases and assertions here...
// }

// int main() {
//     // Run your tests here...
//     test_Handle_message_from_server();
//     return 0;
// }