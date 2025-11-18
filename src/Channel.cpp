#include "../includes/Channel.hpp"
#include <algorithm>
#include <sys/socket.h>

// Constructor
Channel::Channel(const string& name) : _name(name), _topic(""), _key(""), 
    _userLimit(0), _inviteOnly(false), _topicRestricted(true), 
    _hasKey(false), _hasUserLimit(false) {
}

// Destructor
Channel::~Channel() {
}

// Key management
void Channel::setKey(const string& key) {
    _key = key;
    _hasKey = true;
}

void Channel::removeKey() {
    _key = "";
    _hasKey = false;
}

// User limit management
void Channel::setUserLimit(size_t limit) {
    _userLimit = limit;
    _hasUserLimit = true;
}

void Channel::removeUserLimit() {
    _userLimit = 0;
    _hasUserLimit = false;
}

// Member management
bool Channel::addMember(Client* client) {
    if (!client)
        return false;
    
    // Check user limit
    if (_hasUserLimit && _members.size() >= _userLimit) {
        return false;
    }
    
    pair<set<Client*>::iterator, bool> result = _members.insert(client);
    
    // If this is the first member, make them operator
    if (_members.size() == 1) {
        _operators.insert(client);
    }
    
    return result.second;
}

bool Channel::removeMember(Client* client) {
    if (!client)
        return false;
    
    // Remove from operators if they are one
    _operators.erase(client);
    
    return _members.erase(client) > 0;
}

bool Channel::hasMember(Client* client) const {
    return _members.find(client) != _members.end();
}

bool Channel::hasMemberByNick(const string& nickname) const {
    for (set<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        if ((*it)->getNickname() == nickname)
            return true;
    }
    return false;
}

Client* Channel::getMemberByNick(const string& nickname) const {
    for (set<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        if ((*it)->getNickname() == nickname)
            return *it;
    }
    return NULL;
}

// Operator management
bool Channel::addOperator(Client* client) {
    if (!client || !hasMember(client))
        return false;
    
    return _operators.insert(client).second;
}

bool Channel::removeOperator(Client* client) {
    if (!client)
        return false;
    
    return _operators.erase(client) > 0;
}

bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}

// Invite management
void Channel::addInvite(const string& nickname) {
    _inviteList.insert(nickname);
}

void Channel::removeInvite(const string& nickname) {
    _inviteList.erase(nickname);
}

bool Channel::isInvited(const string& nickname) const {
    return _inviteList.find(nickname) != _inviteList.end();
}



// Get list of members for NAMES reply
string Channel::getMemberList() const {
    string list;
    for (set<Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it) {
        if (!list.empty())
            list += " ";
        
        // Prefix operators with @
        if (isOperator(*it))
            list += "@";
        
        list += (*it)->getNickname();
    }
    return list;
}

// Broadcast message to all channel members
void Channel::broadcast(const string& message, Client* exclude) {
    for (set<Client*>::iterator it = _members.begin(); it != _members.end(); ++it) {
        if (*it != exclude) {
            // Send message to client's output buffer
            string msg = message;
            if (msg.find("\r\n") == string::npos) {
                msg += "\r\n";
            }
            send((*it)->getSocketFd(), msg.c_str(), msg.length(), 0);
        }
    }
}

// Get mode string for MODE reply
string Channel::getModeString() const {
    string modes = "+";
    
    if (_inviteOnly)
        modes += "i";
    if (_topicRestricted)
        modes += "t";
    if (_hasKey)
        modes += "k";
    if (_hasUserLimit)
        modes += "l";
    
    if (modes == "+")
        return "";
    
    return modes;
}
