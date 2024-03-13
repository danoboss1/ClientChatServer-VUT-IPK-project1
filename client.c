#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4567
#define MAXLINE 100

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

// AF_INET = IPV4
// SOCK_DGRAM = UDP 

// void connect_socket(){
//     int family = AF_INET;
//     int type = SOCK_DGRAM;
//     int client_socket = socket(family, type, 0);
//     if (client_socket <= 0) 
//         {
//         perror("ERROR: socket");
//         exit(EXIT_FAILURE);
//         }

//     return;
// }


int main(int argc, char *argv[]){

    printf("Program sa uspesne spusti");

    process_arguments(argc, argv);

    printf("coje");

    // connect_socket();

    char buffer[MAXLINE];
    char *message = "Hello Server";
    struct sockaddr_in servaddr;

    // Clear servaddr
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    printf("Nikde som nic nevytvoril");

    // Create datagram socket
    int family = AF_INET;
    int type = SOCK_DGRAM;
    int client_socket = socket(family, type, 0);
    if (client_socket <= 0) 
        {
        perror("ERROR: socket");
        exit(EXIT_FAILURE);
        }

    printf("Neposlal som nikde nic");

    // Send message to the server
    if (sendto(client_socket, message, strlen(message), 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Error: sendto failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Dostal som sa za sendto");

    // TU JE CHYBA
    // // Receive response from the server
    // if (recvfrom(client_socket, buffer, MAXLINE, 0, NULL, NULL) < 0) {
    //     perror("Error: recvfrom failed");
    //     close(client_socket);
    //     exit(EXIT_FAILURE);
    // }

    // // Print the received response
    // printf("Server response: %s\n", buffer);

    // // Close the socket
    // close(client_socket);
    // return 0;
}