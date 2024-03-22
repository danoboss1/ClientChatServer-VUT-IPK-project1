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
#include <poll.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"


size_t find_null_character_position(const char *str, size_t max_length) {
    size_t length = strnlen(str, max_length); // Určí dĺžku reťazca, ale najviac max_length znakov
    return length;
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
    size_t content_length;


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
            offset += content_length; // to idk ci potrebujem
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
            // REPLY JE DEFINOVANY NA 3 STAVY V ZADANI
            memcpy(&(msg->data.reply.result),buffer, sizeof(uint8_t));
            memcpy(&(msg->data.reply.ref_messageID), buffer+1, sizeof(uint16_t));

            offset += sizeof(uint8_t);
            offset += sizeof(uint16_t);

            content_length = find_null_character_position(buffer + offset, MESSAGE_CONTENT_MAX_LENGTH) + 1;
            memcpy(msg->data.reply.messageContent, buffer + offset, content_length);
            break;
        default:
            break;
            //TU MOZNO NEJAKA CHYBA
    }

}