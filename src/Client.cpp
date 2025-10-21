
#include "../includes/Client.hpp"

void Client::processInputBuffer(string chunk)
{

    // append chunk data into the buffer

    cout << "adding chunk: " << chunk << " to buffer\n";
    _readBuffer.append(chunk);

    // command format -> COMMAND [parameters] [:trailing]
   
    string cmd;

    while (true)
    {
        size_t crlf = _readBuffer.find("\r\n");

       if (crlf == string::npos){
            break;
       }

       cmd = _readBuffer.substr(0, crlf);
       
       _readBuffer.erase(0, crlf + 2);
       
       cout << "cmd: " << cmd << endl;
       
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