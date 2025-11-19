
#pragma once

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <map>
#include <vector>
#include <poll.h>
#include <exception>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <csignal>
#include "Client.hpp"
#include "Command.hpp"
#include <sstream>
#include <iomanip>

// Forward declaration
class Channel;

#define BUFFER_SIZE 1024

using namespace std;

class Server
{
private:
    int _port;
    string _serverPassword;
    int _listeningSocketFd;         // server listening socket
    const string _serverName;
    vector<struct pollfd> _pollFds; // vector of file descriptors
    map<int, Client *> _clients;    // map of clients
    static bool signal;
    map<int, Client *> _registeredClients;
    map<string, Channel *> _channels; // IRC channels

public:
    Server(int port, string &password) : _port(port), _serverPassword(password),
                                         _listeningSocketFd(-1),_serverName("IRC SERVER")
    {
    }
    // cpy construcor ...

    // getters / setters
    int getPort() const { return _port; }
    void setPort(int p) { _port = p; }
    const string getServerName(){ return _serverName; }
    std::map<int, Client*>& getClients() { return _clients; }
    std::map<int, Client*>& getRegisteredClients() { return _registeredClients; }
    std::map<string, Channel*>& getChannels() { return _channels; }
     
    const string &getPassword() const { return _serverPassword; }
    void setPassword(const string &pw) { _serverPassword = pw; }

    int getListeningSocketFd() const { return _listeningSocketFd; }
    void setListeningSocketFd(int fd) { _listeningSocketFd = fd; }

    std::vector<struct pollfd> &getPollFds() { return _pollFds; }

    // Channel management
    Channel* getChannel(const string& name);
    Channel* createChannel(const string& name);
    void removeChannel(const string& name);
    
    // Client lookup
    Client* getClientByNick(const string& nickname);

    // exceptions
    class ServerError : public std::runtime_error
    {
    public:
        ServerError(const std::string &msg) : std::runtime_error(msg) {}
    };

    class SocketError : public ServerError
    {
    public:
        SocketError(const std::string &msg) : ServerError("Socket error: " + msg) {}
    };

    class NetworkError : public ServerError
    {
    public:
        NetworkError(const std::string &msg)
            : ServerError("Network error: " + msg + " (" + std::strerror(errno) + ")") {}
    };

    // functions
    void serverStart();
    void serverRun();
    void addNewClient();
    bool authClient(string &clientPassword);
    void closeFds();
    void receiveData(int fd);
    void removeClient(int fd);
    static void SignalHandler(int signum);

    // handle communication => Server -> Client
    void sendReplay(Client* client, int errorNum, string message);
    void send_raw_data(Client* client, const std::string& data);

};
