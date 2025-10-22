
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"

void Client::passCommand(const Command &cmd)
{
    if (cmd.params.empty())
    {
        cerr << "not enough arguments for PASS command!" << endl;
        return;
    }
    else if (cmd.params.size() < 1)
    {
        // send to client -> 461 "PASS :Not enough parameters"
        cerr << "not enough parameters\n";
        return;
    }
    else if (_isAuthenticated == true)
    {
        // send to client -> 462, "You may not reregister"
        cerr << "You are alreayd registered :D\n";
        return;
    }

    this->setPassword(cmd.params[0]);

    cout << "password is set to: " << getPassword() << endl;
}

void Client::nickCommand(const Command &cmd)
{
    if (cmd.params.empty())
    {
        cerr << "not enough arguments for PASS command!" << endl;
        return;
    }
    else if (cmd.params.size() < 1)
    {
        // send to client -> ERR_NONICKNAMEGIVEN
        cerr << "not enough parameters\n";
        return;
    }
    else if (checkUniqueNickname(cmd.params[0]) == false)
    {
        // 433,  ":Nickname is already in use"
        cerr << "Nickname is already in use" << endl;
        return;
    }

    this->setNickname(cmd.params[0]);

    cout << "nickname is set to: " << getNickname() << endl;
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

// void Client::userCommand(const Command& cmd){

// }

// TODO :
//  work on  USER command
// add sender to send errors and logs to the client via the socket :D
//  test if everything is working fine
//  finitoo
