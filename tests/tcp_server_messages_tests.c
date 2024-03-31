#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "../tcp.h"

#include <assert.h>
    

void test_REPLY_OK(){
    MessageType msg_type;

    // ^REPLY OK IS (.{1,1400})$
    char test_reply_ok[FULL_MESSAGE_BUFFER + 1] = "REPLY OK IS SKUSKA 1 2 3 IDE";

    // Function Handle_server_messages_tcp prints out messages that should be printed out
    printf("Should print: Success: {MessageContent}\n");
    msg_type = Handle_server_messages_tcp(test_reply_ok);

    assert(msg_type == MESSAGE_TYPE_REPLY);
}

void test_REPLY_NOK(){
    MessageType msg_type;

    // ^REPLY NOK IS (.{1,1400})$
    char test_reply_nok[FULL_MESSAGE_BUFFER + 1] = "REPLY NOK IS SKUSKA 1 2 3 IDE";

    // Function Handle_server_messages_tcp prints out messages that should be printed out
    printf("Should print: Failure: {MessageContent}\n");
    msg_type = Handle_server_messages_tcp(test_reply_nok);

    assert(msg_type == MESSAGE_TYPE_NOT_REPLY);
}

void test_MSG(){
    MessageType msg_type;

    // ^MSG FROM ([!-~]{1,20}) IS (.{1,1400})$
    char test_message[FULL_MESSAGE_BUFFER + 1] = "MSG FROM DISPLAYNAME IS SKUSKA 1 2 3 IDE";

    // Function Handle_server_messages_tcp prints out messages that should be printed out
    printf("Should print: {DisplayName}: {MessageContent}\n");
    msg_type = Handle_server_messages_tcp(test_message);

    assert(msg_type == MESSAGE_TYPE_MSG);
}

void test_ERR(){
    MessageType msg_type;

    // ^ERR FROM ([!-~]{1,20}) IS (.{1,1400})$
    char test_error[FULL_MESSAGE_BUFFER + 1] = "ERR FROM DISPLAYNAME IS SKUSKA 1 2 3 IDE";

    // Function Handle_server_messages_tcp prints out messages that should be printed out
    printf("Should print: ERR FROM {DisplayName}: {MessageContent}\n");
    msg_type = Handle_server_messages_tcp(test_error);

    assert(msg_type == MESSAGE_TYPE_ERR);
}

void test_BYE(){
    MessageType msg_type;

    // ^BYE
    char test_bye[FULL_MESSAGE_BUFFER + 1] = "BYE";

    // Function Handle_server_messages_tcp prints out messages that should be printed out
    // Do not print anything with BYE message from server
    msg_type = Handle_server_messages_tcp(test_bye);

    assert(msg_type == MESSAGE_TYPE_BYE);
}

// main function for calling the tests
int main() {

    test_REPLY_OK();
    test_REPLY_NOK();
    test_MSG();
    test_ERR();
    test_BYE();

    return 0;
}