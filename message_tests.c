#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "client.h"

#include <assert.h>

// Define your functions for sending messages according to the protocol
// For simplicity, let's assume we have a function send_message() for sending messages


// void test_REPLY() {
//     uint16_t message_id = 456;
//     uint16_t ref_message_id = 789;
//     const char* message_contents = "Success";
//     const char* result = send_message(0x01, message_id, "1", ref_message_id, message_contents);
//     printf("Result: %s\n", result);
// }

// Implement tests for other message types similarly

int main() {
    // Create a Message structure for the confirm message
    Message confirm_message;

    // Populate the confirm_message fields
    confirm_message.type = 0x00; // CONFIRM message type
    uint16_t ref_message_id = 123; // Reference message ID

    // Set the message ID
    confirm_message.messageID = ref_message_id;

    // Calculate the size of the message in the buffer
    size_t message_size = sizeof(uint8_t) + sizeof(uint16_t);

    // Create a buffer to hold the message
    char buffer[message_size];

    // Pack the message into the buffer
    buffer[0] = confirm_message.type;
    memcpy(buffer + 1, &(confirm_message.messageID), sizeof(uint16_t));
    

    Message received_message;

    // Parse the received message buffer using Handle_message_from_server
    size_t bytes_processed = Handle_message_from_server(buffer, &received_message);

    // Now you can use the 'received_message' structure to access the parsed data
    // For example, printing the type and messageID
    printf("Type: %d\n", received_message.type);
    printf("Message ID: %d\n", received_message.messageID);

    return 0;
}

// // test_handle_message.c

// #include "handle_message.h"
// #include <assert.h>

// // Write your test cases here...
// void test_Handle_message_from_server() {
//     // Define your test cases and assertions here...
// }

// int main() {
//     // Run your tests here...
//     test_Handle_message_from_server();
//     return 0;
// }