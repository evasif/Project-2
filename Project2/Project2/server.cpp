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

// 1. create a socket - Get the file descriptor!
// 2. bind to an address - What port am I on?
// 3. listen on a port, and wait for a connection to be established.
// 4. accept the connection from a client.
// 5. send/recv - the same way we read and write for a file.
// 6. shutdown to end read/write.
// 7. close to releases data.


// Function for finding 3 ports in a row that we can use, returns an vector containing the ports that we can use
vector<int> check_open_ports () {

    // Creating variables
    struct sockaddr_in server_addr;
    int sock_fd, connected;
    struct hostent *server;
    int port = 3000;
    int count = 1;
    vector<int> ports;


    // While loops which loops until we find 3 ports in a row we can use
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

            // If the ports is closed, we can use it because if the port is open somebody else is using it
            // Add one to to count until we have count of 3, which means we have 3 ports in a row
            count++;

            // Check if the next port is available
            port++;

            close(sock_fd);
        } else {

            // If a port is open, we assume somebody else is using it and put the counter to 0,
            // because we have to restart finding 3 in a row
            count = 0;

            // Check if the next port is available
            port++;
        }
    }


    // Pushing the available ports into the vector
    for(int i = 0; i < 3; i++) {
        ports.push_back(port-3);
        port++;
    }

    // Returning the vector
    return ports;
}

// Function to read from the client
int read(int fdes, int new_sock_fd) {

    // Creating variables
    char buffer[512];
    int n;

    // Reading from the client
    n = read (fdes, buffer, 512);

    // Error check when reading from the client
    if (n < 0) {
        perror ("Error on read");
        exit (-1);
    }

    // If n is 0 then we have reached the end of the file, and return -1 to stop reading
    else if (n == 0) {
        return -1;
    }

    // If n is larger than 0 we are still reading from the client
    else {

        // Print the message from client to terminal
        cout << "Got message from client: " << buffer << endl;

        // Letting the client know that the server got the message
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

    // For loop to traverse our ports that are available
    // (does not work) only opens the first port in the vector
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

        // Listen to the incoming connections and places all incoming connection into a backlog queue
        listen(sock_fd, 5);

        // Initialize the set of active sockets.
        FD_ZERO(&active_fd_set);
        FD_SET(sock_fd, &active_fd_set);

        cout << "The server is up, you can now send the server a message! " << endl;

        while (true) {

            // Block until input arrives on one or more active sockets.
            read_fd_set = active_fd_set;

            // Select is monitoring multiple file descriptors,
            // waiting until one or more of the file descriptors become ready
            if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
                perror("Error on select");
                exit(-1);
            }

            // For loop for servicing all the sockets with input pending
            for (int j = 0; j < FD_SETSIZE; j++) {

                // Checking if something happened on the socket, it something happened
                // than there is an incoming connection
                if (FD_ISSET(j, &read_fd_set)) {

                    if (j == sock_fd) {

                        // Size of the client address
                        clilen = sizeof(cli_addr);


                        // Extracting the first connection request on the queue of pending connections
                        // for the listening socket, sock_fd, creates a new connected socket, and returns a new file
                        // descriptor referring to that socket.
                        new_sock_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &clilen);

                        // Error check for accept
                        if (new_sock_fd < 0) {
                            perror("Error on accept");
                            exit(-1);
                        }

                        // Printing to terminal when we get a connection from a client
                        cout << "server: got connection from " << inet_ntoa(cli_addr.sin_addr) << " port "
                             << ntohs(cli_addr.sin_port) << endl;

                        // Adds the new_sock_fd to the active_fd_set
                        FD_SET(new_sock_fd, &active_fd_set);

                    // If there is not a new connection incoming we read from the socket
                    } else {

                        // Calling our read() function until we are done reading from the socket
                        if (read(j, new_sock_fd) < 0)
                        {
                            // Close the socket
                             close (j);

                            // Removing the socket from the active_fd_set
                            FD_CLR (j, &active_fd_set);
                        }
                    }
                }
            }
        }
    }

    // Close sockets
    close(sock_fd);
    close(new_sock_fd);
    return 0;
}
