#include "../includes/Commands.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

CommandHandler::CommandHandler(Server* server) : _server(server) {
}

CommandHandler::~CommandHandler() {
}

// Split string by delimiter
vector<string> CommandHandler::split(const string& str, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    
    while (getline(tokenStream, token, delimiter)) {
        if (!token.empty())
            tokens.push_back(token);
    }
    return tokens;
}

// Convert string to uppercase
string CommandHandler::toUpper(const string& str) {
    string result = str;
    for (size_t i = 0; i < result.length(); ++i) {
        result[i] = toupper(result[i]);
    }
    return result;
}

// Check if channel name is valid (starts with # or &)
bool CommandHandler::isValidChannelName(const string& name) {
    return !name.empty() && (name[0] == '#' || name[0] == '&');
}

// Parse and handle incoming command
void CommandHandler::handleCommand(Client* client, const string& message) {
    if (message.empty())
        return;
    
    // Parse command and parameters
    vector<string> tokens = split(message, ' ');
    if (tokens.empty())
        return;
    
    string command = toUpper(tokens[0]);
    vector<string> params(tokens.begin() + 1, tokens.end());
    
    // Route to appropriate handler
    if (command == "KICK")
        handleKick(client, params);
    else if (command == "INVITE")
        handleInvite(client, params);
    else if (command == "TOPIC")
        handleTopic(client, params);
    else if (command == "MODE")
        handleMode(client, params);
    else if (command == "PASS")
        handlePass(client, params);
    else if (command == "NICK")
        handleNick(client, params);
    else if (command == "USER")
        handleUser(client, params);
    else if (command == "JOIN")
        handleJoin(client, params);
    else if (command == "PRIVMSG")
        handlePrivmsg(client, params);
    else if (command == "PART")
        handlePart(client, params);
    else if (command == "QUIT")
        handleQuit(client, params);
    else {
        // Unknown command
        string reply = formatReply(ERR::ERR_UNKNOWNCOMMAND, client->getNickname(), 
                                   command + " :Unknown command");
        client->sendMessage(reply);
    }
}

// ==================== OPERATOR COMMANDS ====================

// KICK <channel> <user> [<comment>]
void CommandHandler::handleKick(Client* client, vector<string>& params) {
    // Check if client is registered
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    // Check parameters
    if (params.size() < 2) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "KICK :Not enough parameters"));
        return;
    }
    
    string channelName = params[0];
    string targetNick = params[1];
    string reason = "No reason given";
    
    // Extract kick reason if provided
    if (params.size() >= 3) {
        reason = "";
        for (size_t i = 2; i < params.size(); ++i) {
            if (i > 2)
                reason += " ";
            reason += params[i];
        }
        // Remove leading : if present
        if (!reason.empty() && reason[0] == ':')
            reason = reason.substr(1);
    }
    
    // Check if channel exists
    Channel* channel = _server->getChannel(channelName);
    if (!channel) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       channelName + " :No such channel"));
        return;
    }
    
    // Check if client is on the channel
    if (!channel->hasMember(client)) {
        client->sendMessage(formatReply(ERR::ERR_NOTONCHANNEL, client->getNickname(), 
                                       channelName + " :You're not on that channel"));
        return;
    }
    
    // Check if client is operator
    if (!channel->isOperator(client)) {
        client->sendMessage(formatReply(ERR::ERR_CHANOPRIVSNEEDED, client->getNickname(), 
                                       channelName + " :You're not channel operator"));
        return;
    }
    
    // Find target client
    Client* target = channel->getMemberByNick(targetNick);
    if (!target) {
        client->sendMessage(formatReply(ERR::ERR_USERNOTINCHANNEL, client->getNickname(), 
                                       targetNick + " " + channelName + " :They aren't on that channel"));
        return;
    }
    
    // Send KICK message to all channel members
    string kickMsg = client->getPrefix() + " KICK " + channelName + " " + 
                     targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, NULL);
    
    // Remove target from channel
    channel->removeMember(target);
    
    // If channel is empty, delete it
    if (channel->getMemberCount() == 0) {
        _server->removeChannel(channelName);
    }
}

// INVITE <nickname> <channel>
void CommandHandler::handleInvite(Client* client, vector<string>& params) {
    // Check if client is registered
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    // Check parameters
    if (params.size() < 2) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "INVITE :Not enough parameters"));
        return;
    }
    
    string targetNick = params[0];
    string channelName = params[1];
    
    // Check if channel exists
    Channel* channel = _server->getChannel(channelName);
    if (!channel) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       channelName + " :No such channel"));
        return;
    }
    
    // Check if client is on the channel
    if (!channel->hasMember(client)) {
        client->sendMessage(formatReply(ERR::ERR_NOTONCHANNEL, client->getNickname(), 
                                       channelName + " :You're not on that channel"));
        return;
    }
    
    // Check if client is operator (required for invite-only channels)
    if (channel->isInviteOnly() && !channel->isOperator(client)) {
        client->sendMessage(formatReply(ERR::ERR_CHANOPRIVSNEEDED, client->getNickname(), 
                                       channelName + " :You're not channel operator"));
        return;
    }
    
    // Find target client
    Client* target = _server->getClientByNick(targetNick);
    if (!target) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHNICK, client->getNickname(), 
                                       targetNick + " :No such nick/channel"));
        return;
    }
    
    // Check if target is already on channel
    if (channel->hasMember(target)) {
        client->sendMessage(formatReply(ERR::ERR_USERONCHANNEL, client->getNickname(), 
                                       targetNick + " " + channelName + " :is already on channel"));
        return;
    }
    
    // Add to invite list
    channel->addInvite(targetNick);
    
    // Send confirmation to inviter
    client->sendMessage(formatReply(RPL::RPL_INVITING, client->getNickname(), 
                                    targetNick + " " + channelName));
    
    // Send INVITE message to target
    string inviteMsg = client->getPrefix() + " INVITE " + targetNick + " :" + channelName + "\r\n";
    target->sendMessage(inviteMsg);
}

// TOPIC <channel> [<topic>]
void CommandHandler::handleTopic(Client* client, vector<string>& params) {
    // Check if client is registered
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    // Check parameters
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "TOPIC :Not enough parameters"));
        return;
    }
    
    string channelName = params[0];
    
    // Check if channel exists
    Channel* channel = _server->getChannel(channelName);
    if (!channel) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       channelName + " :No such channel"));
        return;
    }
    
    // Check if client is on the channel
    if (!channel->hasMember(client)) {
        client->sendMessage(formatReply(ERR::ERR_NOTONCHANNEL, client->getNickname(), 
                                       channelName + " :You're not on that channel"));
        return;
    }
    
    // If no topic parameter, show current topic
    if (params.size() == 1) {
        if (channel->getTopic().empty()) {
            client->sendMessage(formatReply(RPL::RPL_NOTOPIC, client->getNickname(), 
                                           channelName + " :No topic is set"));
        } else {
            client->sendMessage(formatChannelReply(RPL::RPL_TOPIC, client->getNickname(), 
                                                   channelName, ":" + channel->getTopic()));
        }
        return;
    }
    
    // Setting topic - check if topic is restricted and client is operator
    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        client->sendMessage(formatReply(ERR::ERR_CHANOPRIVSNEEDED, client->getNickname(), 
                                       channelName + " :You're not channel operator"));
        return;
    }
    
    // Build new topic from remaining parameters
    string newTopic = "";
    for (size_t i = 1; i < params.size(); ++i) {
        if (i > 1)
            newTopic += " ";
        newTopic += params[i];
    }
    
    // Remove leading : if present
    if (!newTopic.empty() && newTopic[0] == ':')
        newTopic = newTopic.substr(1);
    
    // Set new topic
    channel->setTopic(newTopic);
    
    // Broadcast topic change to all channel members
    string topicMsg = client->getPrefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    channel->broadcast(topicMsg, NULL);
}

// MODE <channel> [<modestring> [<mode arguments>]]
void CommandHandler::handleMode(Client* client, vector<string>& params) {
    // Check if client is registered
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    // Check parameters
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "MODE :Not enough parameters"));
        return;
    }
    
    string target = params[0];
    
    // Only handle channel modes (starts with # or &)
    if (!isValidChannelName(target)) {
        // User mode not implemented
        return;
    }
    
    Channel* channel = _server->getChannel(target);
    if (!channel) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       target + " :No such channel"));
        return;
    }
    
    // If no mode string, show current modes
    if (params.size() == 1) {
        string modeStr = channel->getModeString();
        if (modeStr.empty())
            modeStr = "+";
        client->sendMessage(formatChannelReply(RPL::RPL_CHANNELMODEIS, client->getNickname(), 
                                               target, modeStr));
        return;
    }
    
    // Setting modes - check if client is on channel and is operator
    if (!channel->hasMember(client)) {
        client->sendMessage(formatReply(ERR::ERR_NOTONCHANNEL, client->getNickname(), 
                                       target + " :You're not on that channel"));
        return;
    }
    
    if (!channel->isOperator(client)) {
        client->sendMessage(formatReply(ERR::ERR_CHANOPRIVSNEEDED, client->getNickname(), 
                                       target + " :You're not channel operator"));
        return;
    }
    
    handleChannelMode(client, channel, params);
}

void CommandHandler::handleChannelMode(Client* client, Channel* channel, vector<string>& params) {
    string modeStr = params[1];
    vector<string> modeParams(params.begin() + 2, params.end());
    size_t paramIndex = 0;
    
    applyModeChanges(client, channel, modeStr, modeParams, paramIndex);
}

void CommandHandler::applyModeChanges(Client* client, Channel* channel, const string& modeStr, 
                                     vector<string>& modeParams, size_t& paramIndex) {
    bool adding = true;
    string appliedModes = "";
    string appliedParams = "";
    
    for (size_t i = 0; i < modeStr.length(); ++i) {
        char mode = modeStr[i];
        
        if (mode == '+') {
            adding = true;
            continue;
        } else if (mode == '-') {
            adding = false;
            continue;
        }
        
        // Handle each mode
        switch (mode) {
            case 'i': // Invite-only
                channel->setInviteOnly(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "i";
                break;
                
            case 't': // Topic restricted
                channel->setTopicRestricted(adding);
                appliedModes += (adding ? "+" : "-");
                appliedModes += "t";
                break;
                
            case 'k': // Channel key
                if (adding) {
                    if (paramIndex < modeParams.size()) {
                        channel->setKey(modeParams[paramIndex]);
                        appliedModes += "+k";
                        appliedParams += " " + modeParams[paramIndex];
                        paramIndex++;
                    }
                } else {
                    channel->removeKey();
                    appliedModes += "-k";
                }
                break;
                
            case 'o': // Operator privilege
                if (paramIndex < modeParams.size()) {
                    string targetNick = modeParams[paramIndex];
                    Client* target = channel->getMemberByNick(targetNick);
                    
                    if (target) {
                        if (adding) {
                            channel->addOperator(target);
                        } else {
                            channel->removeOperator(target);
                        }
                        appliedModes += (adding ? "+" : "-");
                        appliedModes += "o";
                        appliedParams += " " + targetNick;
                    }
                    paramIndex++;
                }
                break;
                
            case 'l': // User limit
                if (adding) {
                    if (paramIndex < modeParams.size()) {
                        istringstream iss(modeParams[paramIndex]);
                        size_t limit;
                        if (iss >> limit && limit > 0) {
                            channel->setUserLimit(limit);
                            appliedModes += "+l";
                            appliedParams += " " + modeParams[paramIndex];
                        }
                        paramIndex++;
                    }
                } else {
                    channel->removeUserLimit();
                    appliedModes += "-l";
                }
                break;
                
            default:
                // Unknown mode, ignore
                break;
        }
    }
    
    // Broadcast mode changes to channel if any modes were applied
    if (!appliedModes.empty()) {
        string modeMsg = client->getPrefix() + " MODE " + channel->getName() + 
                        " " + appliedModes + appliedParams + "\r\n";
        channel->broadcast(modeMsg, NULL);
    }
}

// ==================== BASIC IRC COMMANDS ====================

void CommandHandler::handlePass(Client* client, vector<string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_ALREADYREGISTERED, client->getNickname(), 
                                       ":You may not reregister"));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, "*", "PASS :Not enough parameters"));
        return;
    }
    
    string password = params[0];
    if (password[0] == ':')
        password = password.substr(1);
    
    if (_server->authClient(password)) {
        client->setHasPassword(true);
    } else {
        client->sendMessage(formatReply(ERR::ERR_PASSWDMISMATCH, "*", ":Password incorrect"));
    }
}

void CommandHandler::handleNick(Client* client, vector<string>& params) {
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NONICKNAMEGIVEN, "*", ":No nickname given"));
        return;
    }
    
    string newNick = params[0];
    
    // Check if nickname is already in use
    if (_server->getClientByNick(newNick)) {
        client->sendMessage(formatReply(ERR::ERR_NICKNAMEINUSE, "*", newNick + " :Nickname is already in use"));
        return;
    }
    
    string oldNick = client->getNickname();
    client->setNickname(newNick);
    
    // If not registered yet, don't broadcast
    if (oldNick.empty()) {
        return;
    }
    
    // Broadcast nick change
    string nickMsg = ":" + oldNick + " NICK :" + newNick + "\r\n";
    client->sendMessage(nickMsg);
}

void CommandHandler::handleUser(Client* client, vector<string>& params) {
    if (client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_ALREADYREGISTERED, client->getNickname(), 
                                       ":You may not reregister"));
        return;
    }
    
    if (params.size() < 4) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, "*", "USER :Not enough parameters"));
        return;
    }
    
    client->setUsername(params[0]);
    client->setHostname(params[1]);
    
    // Realname starts with : typically
    string realname = params[3];
    for (size_t i = 4; i < params.size(); ++i) {
        realname += " " + params[i];
    }
    if (!realname.empty() && realname[0] == ':')
        realname = realname.substr(1);
    client->setRealname(realname);
    
    // Send welcome if fully registered
    if (client->isRegistered()) {
        sendWelcome(client);
    }
}

void CommandHandler::sendWelcome(Client* client) {
    string welcome = formatReply(RPL::RPL_WELCOME, client->getNickname(), 
                                 ":Welcome to the IRC Network " + client->getPrefix());
    client->sendMessage(welcome);
}

void CommandHandler::handleJoin(Client* client, vector<string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "JOIN :Not enough parameters"));
        return;
    }
    
    string channelName = params[0];
    string key = params.size() > 1 ? params[1] : "";
    
    if (!isValidChannelName(channelName)) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       channelName + " :No such channel"));
        return;
    }
    
    Channel* channel = _server->getChannel(channelName);
    bool isNew = (channel == NULL);
    
    if (isNew) {
        channel = _server->createChannel(channelName);
    }
    
    // Check if already on channel
    if (channel->hasMember(client)) {
        return;
    }
    
    // Check invite-only
    if (channel->isInviteOnly() && !channel->isInvited(client->getNickname())) {
        client->sendMessage(formatReply(ERR::ERR_INVITEONLYCHAN, client->getNickname(), 
                                       channelName + " :Cannot join channel (+i)"));
        return;
    }
    
    // Check key
    if (channel->hasKey() && channel->getKey() != key) {
        client->sendMessage(formatReply(ERR::ERR_BADCHANNELKEY, client->getNickname(), 
                                       channelName + " :Cannot join channel (+k)"));
        return;
    }
    
    // Check user limit
    if (channel->hasUserLimit() && channel->getMemberCount() >= channel->getUserLimit()) {
        client->sendMessage(formatReply(ERR::ERR_CHANNELISFULL, client->getNickname(), 
                                       channelName + " :Cannot join channel (+l)"));
        return;
    }
    
    // Add to channel
    channel->addMember(client);
    channel->removeInvite(client->getNickname());
    
    // Broadcast JOIN
    string joinMsg = client->getPrefix() + " JOIN :" + channelName + "\r\n";
    channel->broadcast(joinMsg, NULL);
    
    // Send topic
    if (!channel->getTopic().empty()) {
        client->sendMessage(formatChannelReply(RPL::RPL_TOPIC, client->getNickname(), 
                                               channelName, ":" + channel->getTopic()));
    }
    
    // Send NAMES
    string names = channel->getMemberList();
    client->sendMessage(formatChannelReply(RPL::RPL_NAMREPLY, client->getNickname(), 
                                           "= " + channelName, ":" + names));
    client->sendMessage(formatChannelReply(RPL::RPL_ENDOFNAMES, client->getNickname(), 
                                           channelName, ":End of /NAMES list"));
}

void CommandHandler::handlePrivmsg(Client* client, vector<string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NORECIPIENT, client->getNickname(), 
                                       ":No recipient given (PRIVMSG)"));
        return;
    }
    
    if (params.size() < 2) {
        client->sendMessage(formatReply(ERR::ERR_NOTEXTTOSEND, client->getNickname(), 
                                       ":No text to send"));
        return;
    }
    
    string target = params[0];
    string message = "";
    for (size_t i = 1; i < params.size(); ++i) {
        if (i > 1)
            message += " ";
        message += params[i];
    }
    if (!message.empty() && message[0] == ':')
        message = message.substr(1);
    
    // Check if target is channel
    if (isValidChannelName(target)) {
        Channel* channel = _server->getChannel(target);
        if (!channel) {
            client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                           target + " :No such channel"));
            return;
        }
        
        if (!channel->hasMember(client)) {
            client->sendMessage(formatReply(ERR::ERR_CANNOTSENDTOCHAN, client->getNickname(), 
                                           target + " :Cannot send to channel"));
            return;
        }
        
        string msg = client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcast(msg, client);
    } else {
        // Private message to user
        Client* targetClient = _server->getClientByNick(target);
        if (!targetClient) {
            client->sendMessage(formatReply(ERR::ERR_NOSUCHNICK, client->getNickname(), 
                                           target + " :No such nick/channel"));
            return;
        }
        
        string msg = client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        targetClient->sendMessage(msg);
    }
}

void CommandHandler::handlePart(Client* client, vector<string>& params) {
    if (!client->isRegistered()) {
        client->sendMessage(formatReply(ERR::ERR_NOTREGISTERED, "*", ":You have not registered"));
        return;
    }
    
    if (params.empty()) {
        client->sendMessage(formatReply(ERR::ERR_NEEDMOREPARAMS, client->getNickname(), 
                                       "PART :Not enough parameters"));
        return;
    }
    
    string channelName = params[0];
    string reason = params.size() > 1 ? params[1] : "";
    if (!reason.empty() && reason[0] == ':')
        reason = reason.substr(1);
    
    Channel* channel = _server->getChannel(channelName);
    if (!channel) {
        client->sendMessage(formatReply(ERR::ERR_NOSUCHCHANNEL, client->getNickname(), 
                                       channelName + " :No such channel"));
        return;
    }
    
    if (!channel->hasMember(client)) {
        client->sendMessage(formatReply(ERR::ERR_NOTONCHANNEL, client->getNickname(), 
                                       channelName + " :You're not on that channel"));
        return;
    }
    
    // Broadcast PART
    string partMsg = client->getPrefix() + " PART " + channelName;
    if (!reason.empty())
        partMsg += " :" + reason;
    partMsg += "\r\n";
    channel->broadcast(partMsg, NULL);
    
    // Remove from channel
    channel->removeMember(client);
    
    // Delete channel if empty
    if (channel->getMemberCount() == 0) {
        _server->removeChannel(channelName);
    }
}

void CommandHandler::handleQuit(Client* client, vector<string>& params) {
    string reason = "Client quit";
    if (!params.empty()) {
        reason = params[0];
        if (!reason.empty() && reason[0] == ':')
            reason = reason.substr(1);
    }
    
    // Broadcast QUIT to all channels the user is in
    const map<string, Channel*>& channels = _server->getChannels();
    for (map<string, Channel*>::const_iterator it = channels.begin(); it != channels.end(); ++it) {
        if (it->second->hasMember(client)) {
            string quitMsg = client->getPrefix() + " QUIT :" + reason + "\r\n";
            it->second->broadcast(quitMsg, client);
            it->second->removeMember(client);
        }
    }
    
    // Close connection (server should handle this)
}
