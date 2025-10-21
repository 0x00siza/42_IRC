#pragma once

#include <string>
#include <iostream>
#include <vector>

using namespace std;
class Server;
class Command;

class Client
{
private:
    int _socketFd;
    string _password;
    string _nickname;
    string _username;
    string hostname; // IP address of the client
    bool _isAuthenticated;
    std::string _readBuffer;   // data received from the client
    std::string _outputBuffer; //
    Server *server;
public:
    Client(int fd, Server *srv = NULL) : _socketFd(fd),
                     _password(""),
                     _nickname(""), // empty until client sets NICK
                     _username(""), // empty until client sets USER
                     hostname(""),
                     _isAuthenticated(false),
                    server(srv)
    {
    }

    void setServer(Server *srv) { server = srv; }
    
    // socket fd
    int getSocketFd() const { return _socketFd; }
    void setSocketFd(int fd) { _socketFd = fd; }

    // password
    const string &getPassword() const { return _password; }
    void setPassword(const string &pw) { _password = pw; }

    // nickname
    const string &getNickname() const { return _nickname; }
    void setNickname(const string &nick) { _nickname = nick; }

    // username
    const string &getUsername() const { return _username; }
    void setUsername(const string &user) { _username = user; }

    // hostname / ip
    const string &getHostname() const { return hostname; }
    void setHostname(const string &host) { hostname = host; }

    // recieved command buffer
    const string &getReadBuffer() const { return _readBuffer; }
    void setReadBuffer(const string &buffer) { _readBuffer = buffer; }

    // exceptions
    class clientError : public std::runtime_error
    {
    public:
        clientError(const std::string &msg) : std::runtime_error(msg) {}
    };

    void processInputBuffer(string chunk);
    void parseCommand(string &cmd);
    void executeCommand(string &cmd);

    void handlePassCommand(const Command& cmd); 
    void handleNickCommand(const Command& cmd);
    void handleUserCommand(const Command& cmd);
    void handleJoinCommand(const Command& cmd);
    void handlePrivMsgCommand(const Command& cmd);
  
};