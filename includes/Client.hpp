#pragma once


#include <string>


using namespace std;
class Server;

class Client {
    private:
        int _socketFd;
        string _password;
        string _nickname;
        string _username;
        bool _isAuthenticated;


    public:
        Client(int fd) :_socketFd(fd){

        }
};