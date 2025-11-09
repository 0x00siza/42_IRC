#pragma once

#include <string>
#include <vector>
#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Utils.hpp"

using namespace std;

// Command handler class
class CommandHandler {
    private:
        Server* _server;
        
    public:
        CommandHandler(Server* server);
        ~CommandHandler();
        
        // Parse and execute command
        void handleCommand(Client* client, const string& message);
        
        // Operator commands (oopsilon)
        void handleKick(Client* client, vector<string>& params);
        void handleInvite(Client* client, vector<string>& params);
        void handleTopic(Client* client, vector<string>& params);
        void handleMode(Client* client, vector<string>& params);
        
        // Basic IRC commands (for context/testing)
        void handlePass(Client* client, vector<string>& params);
        void handleNick(Client* client, vector<string>& params);
        void handleUser(Client* client, vector<string>& params);
        void handleJoin(Client* client, vector<string>& params);
        void handlePrivmsg(Client* client, vector<string>& params);
        void handlePart(Client* client, vector<string>& params);
        void handleQuit(Client* client, vector<string>& params);
        
    private:
        // Helper functions
        vector<string> split(const string& str, char delimiter);
        string toUpper(const string& str);
        bool isValidChannelName(const string& name);
        void sendWelcome(Client* client);
        
        // Mode helper functions
        void handleChannelMode(Client* client, Channel* channel, vector<string>& params);
        void applyModeChanges(Client* client, Channel* channel, const string& modeStr, 
                             vector<string>& modeParams, size_t& paramIndex);
};
