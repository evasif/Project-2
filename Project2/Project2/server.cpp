#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <iostream>
#include <netdb.h>
#include <vector>

using namespace std;

#define MAX  512

// 1. create a socket - Get the file descriptor!
// 2. bind to an address -What port am I on?
// 3. listen on a port, and wait for a connection to be established.
// 4. accept the connection from a client.
// 5. send/recv - the same way we read and write for a file.
// 6. shutdown to end read/write.
// 7. close to releases data.

vector<int> check_open_ports () {
    int port = 3000;
    int count = 1;

    // Creating variables
    struct sockaddr_in server_addr;
    int sock_fd, connected;
    struct hostent *server;


    while(count < 4) {
        // Create the TCP socket
        sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        // Error check for the socket
        if (sock_fd < 0) {
            perror("socket() error");
            exit(-1);
        }

        // Initialise the socketaddr_in structure
        server_addr.sin_family = AF_INET;

        // Fill in the port number
        server_addr.sin_port = htons(port);

        // Getting the descriptor for the corresponding IP address
        server = gethostbyname("127.0.0.1");

        // Error check for the host
        if (server == NULL) {
            perror("host() error");
            exit(0);
        }

        // Doing a memory copy to put it into the right format (right field in the structure)
        memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

        // Connecting to a remote address
        connected = connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

        // Checking if this specific port is open or closed
        if (connected < 0) {
            cout << "port " << port << " is closed" << endl;
            count++;
            port++;

            close(sock_fd);
        } else {
            cout << "port " << port << " is open" << endl;
            count = 0;
            port++;
            close(sock_fd);
        }
    }

    vector<int> ports;

    for(int i = 1; i < 4; i++) {
        ports.push_back(port-3);
        port++;
    }

    return ports;
}

int read (int fdes, int new_sock_fd) {
    char buffer[MAX];
    int n;

    n = read (fdes, buffer, MAX);
    if (n < 0) {
        /* Read error. */
        perror ("Error on read");
        exit (-1);
    }
    else if (n == 0) {
        /* End-of-file. */
        return -1;
    }

    else {
        /* Data read. */
        fprintf (stderr, "Server: got message: %s\n", buffer);
        n = write(new_sock_fd, "Server got your message", 24);
        return 0;
    }
}


int main(int argc, char *argv[])
{
    // Creating variables
    int sock_fd, new_sock_fd, portno, n, max_sd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    fd_set active_fd_set, read_fd_set;
    vector<int> ports = check_open_ports();


    for (int i = 0; i <  ports.size(); i++) {
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
        portno = ports[i];

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
        // Here the maximum size for the backlog queue is 3
        listen(sock_fd, 1);

        /* Initialize the set of active sockets. */
        FD_ZERO(&active_fd_set);
        FD_SET(sock_fd, &active_fd_set);

        while (true) {

            /* Block until input arrives on one or more active sockets. */
            read_fd_set = active_fd_set;

            if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
                perror("select");
                exit(EXIT_FAILURE);
            }

            /* Service all the sockets with input pending. */
            for (int j = 0; j < FD_SETSIZE; j++) {

                if (FD_ISSET(j, &read_fd_set)) {

                    if (j == sock_fd) {

                        clilen = sizeof(cli_addr);

                        new_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &clilen);

                        if (new_sock_fd < 0) {
                            perror("Error on accept");
                            exit(-1);
                        }

                        cout << "server: got connection from " << inet_ntoa(cli_addr.sin_addr) << " port "
                             << ntohs(cli_addr.sin_port) << endl;

                        FD_SET(new_sock_fd, &active_fd_set);
                    } else {

                        if (read(j, new_sock_fd) < 0)
                        {
                            close (i);
                            FD_CLR (i, &active_fd_set);
                        }

                    }

                }
            }
        }
    }

    close(new_sock_fd);
    close(sock_fd);
    return 0;
}


/* They should be allocated dynamically, since part of the assignment is choosing a port range that isn't already in use.
 * The client can then be told the port range that the server is on, and the sequence, and uses that to connect.
 * try and connect() to the port. For the subsequent two ports, don´t forget to close() them if they´re open... */


//http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
//https://www.bogotobogo.com/cplusplus/sockets_server_client.php
