
#include "../includes/Server.hpp"


void Server::serverStart(){
    std::cout << "server is about to go brrr" << std::endl;
      
    // create a socket :DD
    setListeningSocketFd(socket(AF_INET, SOCK_STREAM, 0));
    if (_listeningSocketFd < 0){
        throw SocketError("Failed to create socket");
    }
        
    // set socket options , the most important is SO_REUSEADDR to avoid
    // "Address already in use" erros when restarting your server quickly :)
    int opt = 1;
    if (setsockopt(_listeningSocketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(_listeningSocketFd);
        throw std::runtime_error("Failed to set socket option SO_REUSEADDR");
    }
    
    // bind = associate the socket with the server's IP add
    struct sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;           // IPv4 address family
    server_addr.sin_addr.s_addr = INADDR_ANY;   // Listen on all available network interfaces
                                                // You could also use inet_addr("127.0.0.1") for localhost only
    server_addr.sin_port = htons(_port);  // Convert port number to network byte order (Host TO Network Short)

    if (bind(_listeningSocketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        close(_listeningSocketFd);
        throw NetworkError("Can't bind socket"); // network error or socket error
    }


    std::cout << "listening on 0.0.0.0:" << _port << " (fd=" << getListeningSocketFd() << ")\n";
    // cant connect to privelieged ports btw so use port >= 1024
    
}

// close fds when finished !!!

bool Server::authClient(string &clientPassword){
            return clientPassword == _serverPassword;
}