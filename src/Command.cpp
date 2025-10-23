
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"

void Client::passCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() < 1)
    {
        // send to client -> 461 "PASS :Not enough parameters"
        cerr << "not enough arguments for PASS command!" << endl;
        return;
    }
    
    if (_isRegistered == true)
    {
        // send to client -> 462, "You may not reregister"
        cerr << "You are alreayd registered :D\n";
        return;
    }

    this->setPassword(cmd.params[0]);
    
    // check if passwords match
    if (this->getPassword() != server->getPassword()){
        cerr << "wrong password!" << endl;
        _isAuthenticated = false;
        return;
    }

    _isAuthenticated = true;
    tryToRegister();
    cout << "password is set to: " << getPassword() << endl;
}

void Client::nickCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() < 1)
    {
        // send to client -> ERR_NONICKNAMEGIVEN
        cerr << "not enough arguments for PASS command!" << endl;
        return;
    }
    else if (cmd.params[0].empty() || cmd.params[0][0] == '#' || cmd.params[0][0] == ':' || cmd.params[0].find(' ') != std::string::npos){
        // 432, newNick, "Erroneous nickname"); // ERR_ERRONEUSNICKNAME
        cerr << "ERR_ERRONEUSNICKNAME" << endl;
        return;
    }
    else if (checkUniqueNickname(cmd.params[0]) == false)
    {
        // 433,  ":Nickname is already in use"
        cerr << "Nickname is already in use" << endl;
        return;
    }

    string newNick = cmd.params[0];
    string oldNick = getNickname();
    
    this->setNickname(cmd.params[0]);
    _isNickSet = true;

    std::cout << "Client " << getSocketFd() << " changed nick to: " << getNickname() << std::endl;
    if (_isRegistered && !oldNick.empty()){
        // broadcast change in channels / clients 
        // Example: ":oldNick!user@host NICK :newNick" ...
        
    }

    tryToRegister();
    
}

bool Client::checkUniqueNickname(string nickname)
{
    map<int, Client *>::iterator it;

    for (it = this->server->getClients().begin(); it != this->server->getClients().end(); it++)
    {
        if (it->second->getNickname() == nickname)
            return false;
    }
    return true;
}

void Client::userCommand(const Command& cmd){

     if (cmd.params.size() < 4) { // Needs username, mode, unused, and realname (trailing)
        // server.sendReply(this, 461, "USER", "Not enough parameters");
        return;
    }
    if (_isUserSet) { // USER can only be sent once
        // server.sendReply(this, 462, "You may not reregister"); // ERR_ALREADYREGISTERED
        return;
    }

    // Extract params
    std::string username = cmd.params[0];
    // string mode = cmd[params.1]; // ignore this shishi
    // string unused = cmd[params.2]; // ignore this shishi
    std::string realname = cmd.params[3]; // Trailing parameter

    setUsername(username);
    setRealname(realname);

    _isUserSet = true; // Mark USER as set

    std::cout << "Client " << getSocketFd() << " set USER: " << getUsername() << " Real Name: " << realname << std::endl;
    
    tryToRegister();
}

void Client::tryToRegister(){
    if (!_isNickSet || !_isUserSet || !_isAuthenticated) {
        return; // Not ready to register yet
    }

    if (_isRegistered) { // already registered get outta hereeee n TwT
        return;
    }

    _isRegistered = true;

    // add to registered clients 
    std::pair<std::map<int, Client *>::iterator, bool> res = server->getRegisteredClients().insert(std::make_pair(this->_socketFd, this));
    if (!res.second){
        cerr << "Client is already registered!" << endl;
    }

    cout << "Client " << getNickname() << " is fully registered!" << endl;

    // send weclome message :D
}

// TODO :
//  work on  USER command
// add sender to send errors and logs to the client via the socket :D
//  test if everything is working fine
//  finitoo
