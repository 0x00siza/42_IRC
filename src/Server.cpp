
#include "../includes/Server.hpp"


bool Server::signal = false;

void Server::SignalHandler(int signum)
{
	(void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::signal = true;
}

void Server::serverStart(){
    // cout << "server is about to go brrr" << endl;
      
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
    
    // note: cant connect to privelieged ports btw so use port >= 1024
    
    // setting up a structure to hold file descriptors for poll()
    struct pollfd listeningPollfd;
    listeningPollfd.fd = _listeningSocketFd; // file descriptor to watch
    listeningPollfd.events = POLLIN;         // Watch for incoming data (new clients) -> set the event to POLLIN for reading data
    listeningPollfd.revents = 0;             // revents is set by poll() to indicate actual events
    _pollFds.push_back(listeningPollfd);

    cout << "listening on 0.0.0.0:" << _port << " (fd=" << getListeningSocketFd() << ")\n";
    serverRun();
}

void Server::serverRun(){

    cout << "waiting for connections..." << endl;

    
    while (signal == false){
        
        int fds_count = poll(&_pollFds[0], _pollFds.size(), -1); // waits for file descriptors to become ready to perform I/O
        if (fds_count == -1){ 

            if (errno == EINTR) {
                // Poll was interrupted by a signal. Loop again to check signals.
                continue; 
            }
            // clean up fds and clients ...
            // closeFds();
            throw ServerError("Poll failed!");
        }
        else if (fds_count == 0){ 
            // no event occured here just continue  
            continue;
        }
        else { // event occured

            for (size_t i = 0; i < _pollFds.size(); i++){
                if (_pollFds[i].revents & POLLIN){ // check if there is any data to read or a new pending connection
                    if (_pollFds[i].fd == _listeningSocketFd) // new incoming connections detected on listening socket
                        addNewClient(); // accept all pending connections
                    
                    else{       // It's an already connected client socket
                        cout << "client is already registered - trying to recieve/send data\n";
                        recieveData();
                    }    
                } 
            }

        }

    }
    closeFds();
}

// accept all pending connections (non-blocking accept loop)
void Server::addNewClient(){

   struct sockaddr_in clientAdd;
   socklen_t clientLen = sizeof(clientAdd);

   while (true){
    int clientFd = accept(_listeningSocketFd, (struct sockaddr*) &clientAdd, &clientLen);
    if (clientFd < 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
            break; // no more pending
        std::perror("accept"); // or throw exception ? -> throw is unecessary here so i will just display the error
        break;
    }

    // set to non-blocking socket
    int flags = fcntl(clientFd, F_GETFL, 0);
    if (flags == -1){
        perror("fcntl(F_GETFL) failed");
        close(clientFd);
    } 
    
    if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1){
        perror("fcntl(F_SETFL, O_NONBLOCK) failed");
        close(clientFd);
    }

    // set close-on-exec
    int fdFlags = fcntl(clientFd, F_GETFD);
    if (fdFlags == -1) {
        perror("fcntl(F_GETFD) failed for FD_CLOEXEC");
    }
    else {
        if (fcntl(clientFd, F_SETFD, fdFlags | FD_CLOEXEC) == -1) {
            perror("fcntl(F_SETFD, FD_CLOEXEC) failed");
        }
    }
    

    // Client *newClient = new Client(clientFd); leak

    Client tempClient(clientFd);
    tempClient.setHostname(inet_ntoa(clientAdd.sin_addr));
    
    // insert new client to map and check for errors
    // or use simply ->  _clients[clientFd] = newClient;
    
    std::pair<std::map<int,Client*>::iterator,bool> res = _clients.insert(std::make_pair(clientFd, &tempClient));
    if (!res.second){
        throw runtime_error("Client already exists!");
    }

    struct pollfd pollFd;
    pollFd.fd = clientFd;
    pollFd.events = POLLIN;        
    pollFd.revents = 0;
    _pollFds.push_back(pollFd);

    cout << "accepted " << tempClient.getHostname() << ":" << ntohs(clientAdd.sin_port)
             << " (fd=" << clientFd << ")\n";

    cout << "==============================\n";
    std::cout << "addNewClient: clientFd=" << clientFd
              << " clients=" << _clients.size()
              << " pollfds=" << _pollFds.size() << std::endl;
    // prepare for next accept
    clientLen = sizeof(clientAdd);

    }
}

bool Server::authClient(string &clientPassword){
            return clientPassword == _serverPassword;
}

void Server::closeFds() {
    // close clients
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        int fd = it->first;
        shutdown(fd, SHUT_RDWR);
        close(fd);
        // delete it->second; // commented bc i store client by values now :D
    }
    _clients.clear();

    // clear poll fds
    _pollFds.clear();

    // close listening socket
    if (_listeningSocketFd >= 0) {
        close(_listeningSocketFd);
        _listeningSocketFd = -1;
    }
}


void Server::recieveData(){

}