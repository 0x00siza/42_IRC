
#include "../includes/Client.hpp"

void Client::parseCommand(string chunk){

    // append chunk data into the buffer

    cout << "adding chunk: " << chunk << " to buffer\n";
    _readBuffer.append(chunk);

    // command format -> COMMAND [parameters] [:trailing]
    // command should end of "\r\n"
    string command;

    while (true){
        size_t end = _readBuffer.find("\r\n");
        size_t fallback_end = _readBuffer.find("\n");

        size_t pos;
        bool is_end;
        if (end != string::npos){ 
            is_end = true;
            pos = end;
        }
        else if (fallback_end != string::npos){ 
            is_end = false;
            pos = fallback_end;
        }
        else
            break;
        
        command = _readBuffer.substr(0, pos);

        // remove the command plus its delimiter from the buffer + it will stop the loop
        _readBuffer.erase(0, pos + (is_end ? 2:1));
        // process command
        cout << "buffer: [" << _readBuffer << "]\n";
        cout << "command: " << command << endl;
        cout << "loop" << endl;

        if (is_end == true)
            cout << "cmd ends with '\r\n'" << endl;
        else
            cout << "cmd ends with '\n'" << endl;

    }

}