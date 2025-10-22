
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
    else if (cmd.params.size() < 1){
        // send to client -> 461 "PASS :Not enough parameters"
        cerr << "not enough parameters\n";
        return;
    }
    else if (_isAuthenticated == true){
        // send to client -> 462, "You may not reregister"
        cerr << "You are alreayd registered :D\n";
        return;
    }

    this->setPassword(cmd.params[0]);

    cout << "password is set to: " << getPassword() << endl;
}

void Client::handleNickCommand(const Command& cmd){

}

// void Client::handleUserCommand(const Command& cmd){

// }

// TODO :
//  work on PASS NICK USER
//  test if everything is working fine
//  finitoo
