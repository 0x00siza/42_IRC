
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"

void Client::processInputBuffer(string chunk)
{

    _readBuffer.append(chunk);
    string cmd;

    while (true)
    {
        size_t crlf = _readBuffer.find("\r\n");

       if (crlf == string::npos){
            break;
       }

       cmd = _readBuffer.substr(0, crlf);
       _readBuffer.erase(0, crlf + 2);

       if (cmd.empty())
            return;
       parseCommand(cmd);
    }
}


void Client::parseCommand(string &cmd){

    
    // command format: [<prefix>] <command> <params>
    
    if (cmd.length() > 510){ // Send an error numeric back, preferably ERR_INPUTTOOLONG (417)
        cerr << "command too long" << endl;
        return;
    }
    
    size_t prefStart = cmd.find(":");
    size_t prefEnd = cmd.find(" ");

    if (prefStart != string::npos){
        string prefix = cmd.substr(prefStart, prefEnd);
        // cout << "prefix: " << prefix << endl;
    }

    size_t pos = 0;
    size_t len = cmd.size();

    // optional prefix
    string prefix;
    if (pos < len && cmd[pos] == ':') {
        size_t sp = cmd.find(' ', pos);
        if (sp == string::npos) return; // malformed
        prefix = cmd.substr(pos + 1, sp - pos - 1);
        pos = sp + 1;
    }

    // skip spaces before command token
    while (pos < len && cmd[pos] == ' ') ++pos;
    if (pos >= len) return;

    // command token
    size_t cmd_end = pos;
    while (cmd_end < len && cmd[cmd_end] != ' ') ++cmd_end;
    string command = cmd.substr(pos, cmd_end - pos);

    // normalize command to uppercase so I dont have to check for lowercase for example: check for nick or NICK...
    for (size_t i = 0; i < command.size(); ++i)
        command[i] = (char)std::toupper((unsigned char)command[i]);
    pos = cmd_end;

    // parameters (handle trailing ':' param)
    std::vector<string> params;
    while (pos < len) {
        while (pos < len && cmd[pos] == ' ') ++pos;
            if (pos >= len) break;
            if (cmd[pos] == ':') {
                params.push_back(cmd.substr(pos + 1));
                break;
        }
        size_t p_end = cmd.find(' ', pos);
        if (p_end == string::npos) p_end = len;
        params.push_back(cmd.substr(pos, p_end - pos));
        pos = p_end;
    }

    // debug :D
    // cout << "parsed: prefix=[" << prefix << "] command=[" << command << "] params=[";
    // for (size_t i = 0; i < params.size(); ++i) {
    //     if (i) cout << ", ";
    //     cout << params[i];
    // }
    // cout << "]" << endl;
    
    
    Command newCmd;
    newCmd.prefix = prefix;
    newCmd.command = command;
    newCmd.params = params;
    
    executeCommand(newCmd);
}

void Client::executeCommand(const Command& cmd){
    
    // user must use PASS with correct password to proceed with registration
    
    // check registration first
    if (!_isRegistered) {
        if (cmd.command == "PASS") {
            passCommand(cmd);
        } else if (cmd.command == "NICK") {
            nickCommand(cmd);
        } else if (cmd.command == "USER") {
            userCommand(cmd);
        } else {
            
            this->server->sendReplay(this, 451, "You have not registered");
            return;
        }
        return;
    }
    
    // proceed with other commands :3
    if (cmd.command == "NICK") // can be used multiple times after registration :D
            nickCommand(cmd);
    else if (cmd.command == "JOIN"){
        cout << "JOIN command hehe" << endl;
    
    }
    else if (cmd.command == "KICK"){
        kickCommand(cmd);
    }
    else if (cmd.command == "INVITE"){
        inviteCommand(cmd);
    }
    else if (cmd.command == "TOPIC"){
        topicCommand(cmd);
    }
    else if (cmd.command == "MODE"){
        modeCommand(cmd);
    }
    else if (cmd.command == "QUIT"){
        // ...
    }
    // sends replies (send/queue response strings terminated with "\r\n").

}