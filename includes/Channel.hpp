#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>
#include "Client.hpp"

using namespace std;

class Channel {
    private:
        string _name;
        string _topic;
        string _key; // Channel password (mode +k)
        size_t _userLimit; // Mode +l
        
        set<Client*> _members; // All members in channel
        set<Client*> _operators; // Channel operators
        set<string> _inviteList; // Invited nicknames (for +i mode)
        
        // Channel modes
        bool _inviteOnly; // +i
        bool _topicRestricted; // +t (only operators can change topic)
        bool _hasKey; // +k (password protected)
        bool _hasUserLimit; // +l
        
    public:
        // Constructor
        Channel(const string& name);
        
        // Destructor
        ~Channel();
        
        // Getters
        const string& getName() const { return _name; }
        const string& getTopic() const { return _topic; }
        const string& getKey() const { return _key; }
        size_t getUserLimit() const { return _userLimit; }
        const set<Client*>& getMembers() const { return _members; }
        const set<Client*>& getOperators() const { return _operators; }
        size_t getMemberCount() const { return _members.size(); }
        
        // Mode getters
        bool isInviteOnly() const { return _inviteOnly; }
        bool isTopicRestricted() const { return _topicRestricted; }
        bool hasKey() const { return _hasKey; }
        bool hasUserLimit() const { return _hasUserLimit; }
        
        // Setters
        void setTopic(const string& topic) { _topic = topic; }
        void setKey(const string& key);
        void removeKey();
        void setUserLimit(size_t limit);
        void removeUserLimit();
        
        // Mode setters
        void setInviteOnly(bool mode) { _inviteOnly = mode; }
        void setTopicRestricted(bool mode) { _topicRestricted = mode; }
        
        // Member management
        bool addMember(Client* client);
        bool removeMember(Client* client);
        bool hasMember(Client* client) const;
        bool hasMemberByNick(const string& nickname) const;
        Client* getMemberByNick(const string& nickname) const;
        
        // Operator management
        bool addOperator(Client* client);
        bool removeOperator(Client* client);
        bool isOperator(Client* client) const;
        
        // Invite management
        void addInvite(const string& nickname);
        void removeInvite(const string& nickname);
        bool isInvited(const string& nickname) const;
        
        // Utility
        void broadcast(const string& message, Client* exclude = NULL);
        string getMemberList() const;
        string getModeString() const;
};