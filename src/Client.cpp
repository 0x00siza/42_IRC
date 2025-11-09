#include "../includes/Client.hpp"
#include <sstream>

// Constructor
Client::Client(int fd) : _socketFd(fd), _nickname(""), _username(""), 
    _realname(""), _hostname(""), _isAuthenticated(false), 
    _hasPassword(false), _hasNickname(false), _hasUsername(false) {
}

// Destructor
Client::~Client() {
}

// Setters
void Client::setNickname(const string& nickname) {
    _nickname = nickname;
    if (!nickname.empty())
        _hasNickname = true;
}

void Client::setUsername(const string& username) {
    _username = username;
    if (!username.empty())
        _hasUsername = true;
}

void Client::setRealname(const string& realname) {
    _realname = realname;
}

void Client::setHostname(const string& hostname) {
    _hostname = hostname;
}

// Buffer management
void Client::appendToReadBuffer(const string& data) {
    _readBuffer += data;
}

void Client::appendToWriteBuffer(const string& data) {
    _writeBuffer += data;
}

// Extract a complete message (ending with \r\n) from the read buffer
string Client::extractMessage() {
    size_t pos = _readBuffer.find("\r\n");
    if (pos == string::npos)
        return "";
    
    string message = _readBuffer.substr(0, pos);
    _readBuffer.erase(0, pos + 2); // Remove message + \r\n
    return message;
}

// Get prefix in IRC format: :nickname!username@hostname
string Client::getPrefix() const {
    string prefix = ":";
    prefix += _nickname.empty() ? "*" : _nickname;
    if (!_username.empty()) {
        prefix += "!";
        prefix += _username;
    }
    if (!_hostname.empty()) {
        prefix += "@";
        prefix += _hostname;
    }
    return prefix;
}

// Add message to write buffer (ensures proper IRC format with \r\n)
void Client::sendMessage(const string& message) {
    _writeBuffer += message;
    if (message.size() < 2 || message.substr(message.size() - 2) != "\r\n") {
        _writeBuffer += "\r\n";
    }
}