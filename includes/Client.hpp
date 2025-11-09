#pragma once

#include <string>
#include <vector>

using namespace std;
class Server;

class Client {
    private:
        int _socketFd;
        string _nickname;
        string _username;
        string _realname;
        string _hostname;
        bool _isAuthenticated;
        bool _hasPassword;
        bool _hasNickname;
        bool _hasUsername;
        string _readBuffer;
        string _writeBuffer;

    public:
        // Constructor
        Client(int fd);
        
        // Destructor
        ~Client();
        
        // Getters
        int getSocketFd() const { return _socketFd; }
        const string& getNickname() const { return _nickname; }
        const string& getUsername() const { return _username; }
        const string& getRealname() const { return _realname; }
        const string& getHostname() const { return _hostname; }
        bool isAuthenticated() const { return _isAuthenticated; }
        bool hasPassword() const { return _hasPassword; }
        bool hasNickname() const { return _hasNickname; }
        bool hasUsername() const { return _hasUsername; }
        bool isRegistered() const { return _hasPassword && _hasNickname && _hasUsername; }
        const string& getReadBuffer() const { return _readBuffer; }
        const string& getWriteBuffer() const { return _writeBuffer; }
        
        // Setters
        void setNickname(const string& nickname);
        void setUsername(const string& username);
        void setRealname(const string& realname);
        void setHostname(const string& hostname);
        void setAuthenticated(bool auth) { _isAuthenticated = auth; }
        void setHasPassword(bool has) { _hasPassword = has; }
        void setHasNickname(bool has) { _hasNickname = has; }
        void setHasUsername(bool has) { _hasUsername = has; }
        
        // Buffer management
        void appendToReadBuffer(const string& data);
        void appendToWriteBuffer(const string& data);
        void clearReadBuffer() { _readBuffer.clear(); }
        void clearWriteBuffer() { _writeBuffer.clear(); }
        string extractMessage(); // Extract complete message from buffer
        
        // Utility
        string getPrefix() const; // Returns :nickname!username@hostname
        void sendMessage(const string& message); // Add message to write buffer
};