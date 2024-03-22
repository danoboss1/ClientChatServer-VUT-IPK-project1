#define DISPLAY_NAME_MAX_LENGTH 20
#define CHANNEL_ID_MAX_LENGTH 20
#define USERNAME_MAX_LENGTH 20  
#define SECRET_MAX_LENGHT 128
#define MESSAGE_CONTENT_MAX_LENGTH 1400

#define FULL_MESSAGE_BUFFER 1500


// Define message types
// uvidim ci toto tu necham
// lebo to robi chybu ked sa snazim priradin 0x00, 0x01 hodnoty
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
    uint8_t type;
    uint16_t messageID;
    union {
        StructAuth auth;
        StructJoin join;
        StructErr err;
        StructMsg msg;
        StructReply reply;
    } data;
} Message;

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