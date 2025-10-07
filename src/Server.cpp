
#include "../includes/Server.hpp"


void Server::serverStart(){
    std::cout << "server is about to go brrr" << std::endl;
      
    // setting up your listening socket
    // create a socket :DD
    setListeningSocketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (_listeningSocketFd < 0){
        throw SocketError("Failed to create socket");
    }
        
    // set socket options , the most important is SO_REUSEADDR to avoid
    // "Address already in use" errors when restarting your server quickly :)
    int opt = 1;
    if (setsockopt(_listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(_listeningSocketFd);
        throw std::runtime_error("Failed to set socket option SO_REUSEADDR");
    }
    
    // bind = associate the socket with the server's IP add + port number
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr)); // set all struct values to 0
    server_addr.sin_family = AF_INET;           // IPv4 address family
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Listen on all available network interfaces
                                                // You could also use inet_addr("127.0.0.1") for localhost only
    server_addr.sin_port = htons(_port);  // Convert port number to network byte order (Host TO Network Short)

    if (bind(_listeningSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(_listeningSocketFd);
        throw NetworkError("Can't bind socket"); // network error or socket error
    }
    // all these lines associated to bind result -> telling the OS to Associate this socket (_listeningSocketFd) 
    // with IPv4 connections that arrive at any of my local IP addresses on the port
    
 
    // Make the socket ready to accept new connections.
    // SOMAXCONN sets the max number of clients waiting to connect (in a queue).
    // This queue holds connections that are finishing their 3-way handshake.
    if (listen(_listeningSocketFd, SOMAXCONN) == -1){
        close(_listeningSocketFd);
        throw SocketError("Failed to start listening on socket");
    }

    // btw network sockets are also treated as file descriptors 
    // Set File Descriptor Flags for socket (e.g., Non-Blocking Mode)
    // Non-Blocking Mode -> make the server able to handle multiple clients
        // without hanging and waiting to send/recieve data for one clinet
    if (fcntl(_listeningSocketFd, F_SETFL, O_NONBLOCK)){
        close(_listeningSocketFd);
        throw SocketError("Failed to start listening on socket");
    }
    
    std::cout << "listening on 0.0.0.0:" << _port << " (fd=" << getListeningSocketFd() << ")\n";
    // cant connect to privelieged ports btw so use port >= 1024
    
}

// close fds when finished !!!

bool Server::authClient(string &clientPassword){
            return clientPassword == _serverPassword;
}