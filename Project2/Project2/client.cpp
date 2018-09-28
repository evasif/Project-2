#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    // Creating variables
    int sock_fd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    // Checking that there is right amount of arguments
    if (argc < 3) {
        perror("Usage: hostname port");
        exit(-1);
    }

    // Giving the portno the port from arguments
    portno = atoi(argv[2]);

    // Create the socket
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Error check for the socket
    if (sock_fd < 0) {
        perror("Error opening socket");
        exit(-1);
    }

    // Giving the the hostname from arguments
    server = gethostbyname(argv[1]);

    // Error check for the host
    if (server == NULL) {
        perror("Error, no such host");
        exit(-1);
    }

    // Clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Initialise the socketaddr_in structure
    serv_addr.sin_family = AF_INET;

    // Clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));

    // Converting port number to network byte order
    serv_addr.sin_port = htons(portno);

    // Connecting to a remote address
    int connected = connect(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    // Error check for connect
    if (connected < 0) {
        perror("Error connecting");
        exit(-1);
    }

    printf("Please enter the message: ");

    // Erases the previous data in the buffer
    bzero(buffer, 256);

    // Reading the message from the client
    fgets(buffer, 255, stdin);

    // Writing to socket
    n = write(sock_fd, buffer, strlen(buffer));

    // Error check for writing to socket
    if (n < 0) {
        perror("Error writing to socket");
        exit(-1);
    }

    // Reading from socket
    bzero(buffer, 256);
    n = read(sock_fd, buffer, 255);

    // Error check for reading from socket
    if (n < 0) {
        perror("Error reading from socket");
        exit(-1);
    }

    printf("%.*s\n",n,buffer);
    close(sock_fd);
    return 0;
}