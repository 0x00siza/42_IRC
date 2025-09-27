// #include "Server.hpp"

// int main(){
//     std::cout << "server is about to go brrr" << std::endl;

//     //  socket() -> asks the kernel to create a socket object
//      // domain: AF_INET  (IPv4)
//     // type:   SOCK_STREAM (TCP - reliable stream)
//     // protocol: 0 -> kernel picks TCP for SOCK_STREAM

//     int fd = socket(AF_INET, SOCK_STREAM, 0);
//     if (fd < 0){
//         std::perror("socket");
//         return 1;
//     }

//     std::cout << fd << std::endl;

//     close(fd);

// }

// ...existing code...
#include <cstring>
#include <cstdio>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;

int main()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(serverSocket);
        return 1;
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind");
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        perror("listen");
        close(serverSocket);
        return 1;
    }

    cout << "listening on 0.0.0.0:8080" << endl;

    sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientSocket < 0) {
        perror("accept");
        close(serverSocket);
        return 1;
    }

    cout << "accepted connection from " << inet_ntoa(clientAddr.sin_addr)
         << ":" << ntohs(clientAddr.sin_port) << endl;

    char buffer[1024];
    ssize_t n;
    while ((n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';
        cout << "Message from client: " << buffer;
        if (buffer[n-1] != '\n') cout << endl;
    }
    if (n == 0) {
        cout << "client closed connection" << endl;
    } else if (n < 0) {
        perror("recv");
    }

    close(clientSocket);
    close(serverSocket);
    return 0;
}
// ...existing code...