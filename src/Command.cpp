
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"

void Client::passCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() != 1)
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }
    
    if (_isRegistered == true)
    {
        server->sendReplay(this, 462, ":You may not reregister");
        return;
    }

    this->setPassword(cmd.params[0]);
    
    if (this->getPassword() != server->getPassword()){
        _isAuthenticated = false;
        server->sendReplay(this, 464, ":Password Incorrect"); // ??
        return;
    }

    _isAuthenticated = true;
    tryToRegister();
    cout << "password is set to: " << getPassword() << endl;
}

void Client::nickCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() < 1 || cmd.params[0].length() > 9)
    {
        server->sendReplay(this, 431, ":No nickname given");
        return;
    }
    else if (cmd.params[0].empty() || cmd.params[0][0] == '#' || cmd.params[0][0] == ':' || cmd.params[0].find(' ') != std::string::npos){
        server->sendReplay(this, 432, "Erroneous nickname");
        return;
    }
    else if (checkUniqueNickname(cmd.params[0]) == false)
    {
        server->sendReplay(this, 433, ":Nickname is already in use");
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
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }
    if (_isUserSet) {
        server->sendReplay(this, 462, ":You may not reregister");
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
        server->sendReplay(this, 462, ":You may not reregister");
    }

    cout << "Client " << getNickname() << " is fully registered!" << endl;

    // send weclome message :D
}
