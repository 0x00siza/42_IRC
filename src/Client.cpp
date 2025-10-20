
#include "../includes/Client.hpp"

void Client::processInputBuffer(string chunk)
{

    // append chunk data into the buffer

    cout << "adding chunk: " << chunk << " to buffer\n";
    _readBuffer.append(chunk);

    // command format -> COMMAND [parameters] [:trailing]
    // command should end of "\r\n"
    string command;

    while (true)
    {
        size_t crlf = _readBuffer.find("\r\n");
        size_t lf = _readBuffer.find("\n");

        size_t pos;
        bool is_delimiter;

        if (crlf == string::npos && lf == string::npos)
            break;

        if (crlf != string::npos && (lf == string::npos || crlf <= lf))
        {
            is_delimiter = true;
            pos = crlf;
        }
        else
        {
            is_delimiter = false;
            pos = lf;
        }

        command = _readBuffer.substr(0, pos);
        std::cout << "command: [" << command << "]\n";
        _readBuffer.erase(0, pos + (is_delimiter ? 2 : 1));

        
        cout << "buffer: [" << _readBuffer << "]\n";

        cout << "last char: " << command[command.size() - 1] << endl;
        if (!command.empty() && command[command.size() - 1] == '\r')
        {
            cout << "here" << endl;
            command.resize(command.size() - 1);
        }
        
        if (command.empty())
            continue;
        
        std::cout << "command: [" << command << "]\n";



        // processCommand();
        // TODO:
        // Enforce limits (max length, allowed chars).
        // Parse the line into: optional prefix, command token, parameters (with trailing param starting with ':') — RFC 2812 style.
        // Normalize the command (uppercase) for dispatch.
        // Dispatch to a handler that:
        // updates per-client state (nickname, registration).
        // validates permissions (only respond to some commands before registration).
        // sends replies (send/queue response strings terminated with "\r\n").
        // broadcasts to channels / other clients if needed.
        // closes the client on protocol error or abuse.
        // Log the command for debugging and remove processed bytes (already done).
        // Repeat loop to handle more complete lines.
    }
}