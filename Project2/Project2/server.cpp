#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// 1. create a socket - Get the file descriptor!
// 2. bind to an address -What port am I on?
// 3. listen on a port, and wait for a connection to be established.
// 4. accept the connection from a client.
// 5. send/recv - the same way we read and write for a file.
// 6. shutdown to end read/write.
// 7. close to releases data.

int main(int argc, char *argv[])
{
    // Creating variables
    int sock_fd, new_sock_fd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    if (argc < 2) {
        perror("No port provided");
        exit(-1);
    }
    
    // Create the socket
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    // Error check for the socket
    if (sock_fd < 0) {
        perror("Error creating socket");
        exit(-1);
    }
    
    // Clear address structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    // Setting the port number to the provided argument
    portno = atoi(argv[1]);
    
    // Initialise the socketaddr_in structure
    serv_addr.sin_family = AF_INET;
    
    // Automatically fill with current host's IP address
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Converting port number to network byte order
    serv_addr.sin_port = htons(portno);
    
    // Binding the socket to the current IP address on port, portno
    int binding = bind(sock_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    
    // Error check for binding
    if (binding < 0) {
        perror("Error binding");
        exit(-1);
    }
    
    // This listen() call tells the socket to listen to the incoming connections and places all incoming
    // connection into a backlog queue until accept() call accepts the connection
    // Here the maximum size for the backlog queue is 5
    listen(sock_fd,5);
    
    // Accepts an incoming connection
    clilen = sizeof(cli_addr);
    
    // This accept() function will write the connecting client's address info
    // into the the address structure and the size of that structure is clilen.
    // The accept() returns a new socket file descriptor for the accepted connection.
    // So, the original socket file descriptor can continue to be used
    // for accepting new connections while the new socker file descriptor is used for
    // communicating with the connected client.
    new_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &clilen);
    
    if (new_sock_fd < 0) {
        perror("Error on accept");
        exit(-1);
    }
    
    printf("server: got connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
    
    
    // This send() function sends the 13 bytes of the string to the new socket
    send(new_sock_fd, "Hello, world!\n", 13, 0);
    
    bzero(buffer,256);
    
    n = read(new_sock_fd,buffer,255);
    
    if (n < 0) {
        perror("Error reading from socket");
        exit(-1);
    }
    printf("Here is the message: %s\n",buffer);
    
    close(new_sock_fd);
    close(sock_fd);
    return 0;
}
