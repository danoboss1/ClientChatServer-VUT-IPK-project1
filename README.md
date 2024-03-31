# Client-for-Chat-server-using-IPK24-CHAT-Protocol
### VUT University : IPK_2024 Project1

## Table of contents
- Task
- Project description
- Usage
    - Compile
    - Run
    - Use
- Technical details
- Licence
- Used sources

## Task
Implement a client for Chat server using IPK24-CHAT Protocol.

## Project description 
This project focuses on creating a client enabling communication with a remote server using the IPK24-CHAT protocol. The project has two variants: TCP and UDP.

## Usage
### Compile
This project can be compiled using Makefile:
```
make
```
### Run
After compilation it can be run using following command:
```
./ipk24chat-client -t <protocol variant> -s <server IP/hostname> -p <port> -d <UDP confirmation timeout> -r <max num of UDP retransmissions> -h <help>
```

### Use

U can start program by the command in the run section. Than u need to verify yourself through `/auth name secret display_name` command. After successful autentification u can change channels with join command `/join channelID` or send message with simply entering message to the terminal `Entered message`. To exit program press `CTRL+C`. All other possible commands and errors handling are descripted here (https://git.fit.vutbr.cz/NESFIT/IPK-Projects-2024/src/branch/master/Project%201)

## Technical details
Program was written in C language.  

## License
The program was made according to GNU General Public Licence v3.0.

## Used sources
- IPK Prezentácie
- IPK Prednášky
- VUT discord
- poll.h documentation* https://pubs.opengroup.org/onlinepubs/009695399/basedefs/poll.h.html
- tcp client* https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
- udp client* https://www.geeksforgeeks.org/udp-client-server-using-connect-c-implementation/

