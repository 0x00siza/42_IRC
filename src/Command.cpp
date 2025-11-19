
#include "Client.hpp"
#include "Command.hpp"
#include "Server.hpp"
#include "Channel.hpp"

void Client::passCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() != 1)
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }

    if (_isRegistered == true)
    {
        server->sendReplay(this, 462, ":You may not reregister");
        return;
    }

    this->setPassword(cmd.params[0]);

    if (this->getPassword() != server->getPassword())
    {
        _isAuthenticated = false;
        server->sendReplay(this, 464, ":Password Incorrect"); // ??
        return;
    }

    _isAuthenticated = true;
    tryToRegister();
    cout << "password is set to: " << getPassword() << endl;
}

void Client::nickCommand(const Command &cmd)
{
    if (cmd.params.empty() || cmd.params.size() < 1 || cmd.params[0].length() > 9)
    {
        server->sendReplay(this, 431, ":No nickname given");
        return;
    }
    else if (cmd.params[0].empty() || cmd.params[0][0] == '#' || cmd.params[0][0] == ':' || cmd.params[0].find(' ') != std::string::npos)
    {
        server->sendReplay(this, 432, "Erroneous nickname");
        return;
    }
    else if (checkUniqueNickname(cmd.params[0]) == false)
    {
        server->sendReplay(this, 433, ":Nickname is already in use");
        return;
    }

    string newNick = cmd.params[0];
    string oldNick = getNickname();

    this->setNickname(cmd.params[0]);
    _isNickSet = true;

    std::cout << "Client " << getSocketFd() << " changed nick to: " << getNickname() << std::endl;
    if (_isRegistered && !oldNick.empty())
    {
        // broadcast change in channels / clients
        // Example: ":oldNick!user@host NICK :newNick" ...
    }

    tryToRegister();
}

bool Client::checkUniqueNickname(string nickname)
{
    map<int, Client *>::iterator it;

    for (it = this->server->getClients().begin(); it != this->server->getClients().end(); it++)
    {
        if (it->second->getNickname() == nickname)
            return false;
    }
    return true;
}

void Client::userCommand(const Command &cmd)
{

    if (cmd.params.size() < 4)
    { // Needs username, mode, unused, and realname (trailing)
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }
    if (_isUserSet)
    {
        server->sendReplay(this, 462, ":You may not reregister");
        return;
    }

    // Extract params
    std::string username = cmd.params[0];
    // string mode = cmd[params.1]; // ignore this shishi
    // string unused = cmd[params.2]; // ignore this shishi
    std::string realname = cmd.params[3]; // Trailing parameter

    setUsername(username);
    setRealname(realname);

    _isUserSet = true; // Mark USER as set

    std::cout << "Client " << getSocketFd() << " set USER: " << getUsername() << " Real Name: " << realname << std::endl;

    tryToRegister();
}

void Client::sendWelcomeMessages()
{
    server->sendReplay(this, 1, ":Welcome to the Internet Relay Network " + getNickname() + "!" + getUsername() + "@" + getHostname());
    server->sendReplay(this, 2, ":Your host is IRC_SERVER, running version 1.0");
    server->sendReplay(this, 3, ":This server was created today");
    server->sendReplay(this, 4, "IRC_SERVER 1.0");
}

void Client::tryToRegister()
{
    if (!_isNickSet || !_isUserSet || !_isAuthenticated)
    {
        return; // Not ready to register yet
    }

    if (_isRegistered)
    { // already registered get outta hereeee n TwT
        return;
    }

    _isRegistered = true;

    // add to registered clients
    std::pair<std::map<int, Client *>::iterator, bool> res = server->getRegisteredClients().insert(std::make_pair(this->_socketFd, this));
    if (!res.second)
    {
        server->sendReplay(this, 462, ":You may not reregister");
    }

    sendWelcomeMessages();
}

// ==================== OPERATOR COMMANDS ====================

void Client::kickCommand(const Command &cmd)
{
    // KICK <channel> <user> [:<reason>]
    if (cmd.params.size() < 2)
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }

    string channelName = cmd.params[0];
    string targetNick = cmd.params[1];
    string reason = cmd.params.size() >= 3 ? cmd.params[2] : this->getNickname();

    // Get channel
    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {
        server->sendReplay(this, 403, channelName + " :No such channel");
        return;
    }

    // Check if kicker is on channel
    if (!channel->hasMember(this))
    {
        server->sendReplay(this, 442, channelName + " :You're not on that channel");
        return;
    }

    // Check if kicker is operator
    if (!channel->isOperator(this))
    {
        server->sendReplay(this, 482, channelName + " :You're not channel operator");
        return;
    }

    // Get target client
    Client *target = channel->getMemberByNick(targetNick);
    if (!target)
    {
        server->sendReplay(this, 441, targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }

    // Broadcast KICK message to all channel members
    string kickMsg = ":" + this->getNickname() + "!" + this->getUsername() + "@localhost KICK " +
                     channelName + " " + targetNick + " :" + reason + "\r\n";
    channel->broadcast(kickMsg, NULL);

    // Remove target from channel
    channel->removeMember(target);
}

void Client::inviteCommand(const Command &cmd)
{
    // INVITE <nickname> <channel>
    if (cmd.params.size() < 2)
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }

    string targetNick = cmd.params[0];
    string channelName = cmd.params[1];

    // Get target client
    Client *target = server->getClientByNick(targetNick);
    if (!target)
    {
        server->sendReplay(this, 401, targetNick + " :No such nick/channel");
        return;
    }

    // Get channel
    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {
        server->sendReplay(this, 403, channelName + " :No such channel");
        return;
    }

    // Check if inviter is on channel
    if (!channel->hasMember(this))
    {
        server->sendReplay(this, 442, channelName + " :You're not on that channel");
        return;
    }

    // Check if inviter is operator (required for +i channels)
    if (channel->isInviteOnly() && !channel->isOperator(this))
    {
        server->sendReplay(this, 482, channelName + " :You're not channel operator");
        return;
    }

    // Check if target is already on channel
    if (channel->hasMember(target))
    {
        server->sendReplay(this, 443, targetNick + " " + channelName + " :is already on channel");
        return;
    }

    // Add to invite list
    channel->addInvite(targetNick);

    // Send RPL_INVITING to inviter
    server->sendReplay(this, 341, targetNick + " " + channelName);

    // Send INVITE message to target
    string inviteMsg = ":" + this->getNickname() + "!" + this->getUsername() + "@localhost INVITE " +
                       targetNick + " :" + channelName;
    server->send_raw_data(target, inviteMsg);
}

void Client::topicCommand(const Command &cmd)
{
    // TOPIC <channel> [:<new topic>]
    if (cmd.params.empty())
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }

    string channelName = cmd.params[0];

    // Get channel
    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {
        server->sendReplay(this, 403, channelName + " :No such channel");
        return;
    }

    // Check if client is on channel
    if (!channel->hasMember(this))
    {
        server->sendReplay(this, 442, channelName + " :You're not on that channel");
        return;
    }

    // If no topic parameter, return current topic
    if (cmd.params.size() == 1)
    {
        if (channel->getTopic().empty())
        {
            server->sendReplay(this, 331, channelName + " :No topic is set");
        }
        else
        {
            server->sendReplay(this, 332, channelName + " :" + channel->getTopic());
        }
        return;
    }

    // Setting a new topic
    string newTopic = cmd.params[1];

    // Check if topic is restricted and client is operator
    if (channel->isTopicRestricted() && !channel->isOperator(this))
    {
        server->sendReplay(this, 482, channelName + " :You're not channel operator");
        return;
    }

    // Set new topic
    channel->setTopic(newTopic);

    // Broadcast topic change to all channel members
    string topicMsg = ":" + this->getNickname() + "!" + this->getUsername() + "@localhost TOPIC " +
                      channelName + " :" + newTopic + "\r\n";
    channel->broadcast(topicMsg, NULL);
}

void Client::modeCommand(const Command &cmd)
{
    // MODE <channel> [<modestring> [<mode arguments>...]]
    if (cmd.params.empty())
    {
        server->sendReplay(this, 461, "Not enough parameters");
        return;
    }

    string channelName = cmd.params[0];

    // Get channel
    Channel *channel = server->getChannel(channelName);
    if (!channel)
    {
        server->sendReplay(this, 403, channelName + " :No such channel");
        return;
    }

    // If no mode string, return current modes
    if (cmd.params.size() == 1)
    {
        string modes = channel->getModeString();
        server->sendReplay(this, 324, channelName + " " + modes);
        return;
    }

    // Check if client is on channel
    if (!channel->hasMember(this))
    {
        server->sendReplay(this, 442, channelName + " :You're not on that channel");
        return;
    }

    // Check if client is operator
    if (!channel->isOperator(this))
    {
        server->sendReplay(this, 482, channelName + " :You're not channel operator");
        return;
    }

    // Parse mode string
    string modeString = cmd.params[1];
    vector<string> modeArgs;
    for (size_t i = 2; i < cmd.params.size(); ++i)
    {
        modeArgs.push_back(cmd.params[i]);
    }

    // Apply modes
    bool adding = true;
    size_t argIndex = 0;
    string appliedModes;
    string appliedArgs;

    for (size_t i = 0; i < modeString.size(); ++i)
    {
        char mode = modeString[i];

        if (mode == '+')
        {
            adding = true;
            continue;
        }
        else if (mode == '-')
        {
            adding = false;
            continue;
        }

        // Process individual modes
        if (mode == 'i')
        {
            channel->setInviteOnly(adding);
            appliedModes += mode;
        }
        else if (mode == 't')
        {
            channel->setTopicRestricted(adding);
            appliedModes += mode;
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (argIndex < modeArgs.size())
                {
                    channel->setKey(modeArgs[argIndex]);
                    appliedModes += mode;
                    appliedArgs += " " + modeArgs[argIndex];
                    argIndex++;
                }
            }
            else
            {
                channel->removeKey();
                appliedModes += mode;
            }
        }
        else if (mode == 'o')
        {
            if (argIndex < modeArgs.size())
            {
                Client *target = channel->getMemberByNick(modeArgs[argIndex]);
                if (target)
                {
                    if (adding)
                        channel->addOperator(target);
                    else
                        channel->removeOperator(target);
                    appliedModes += mode;
                    appliedArgs += " " + modeArgs[argIndex];
                }
                argIndex++;
            }
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (argIndex < modeArgs.size())
                {
                    size_t limit = 0;
                    for (size_t j = 0; j < modeArgs[argIndex].size(); ++j)
                    {
                        if (modeArgs[argIndex][j] >= '0' && modeArgs[argIndex][j] <= '9')
                            limit = limit * 10 + (modeArgs[argIndex][j] - '0');
                    }
                    if (limit > 0)
                    {
                        channel->setUserLimit(limit);
                        appliedModes += mode;
                        appliedArgs += " " + modeArgs[argIndex];
                    }
                    argIndex++;
                }
            }
            else
            {
                channel->removeUserLimit();
                appliedModes += mode;
            }
        }
    }

    // Broadcast mode change to all channel members
    if (!appliedModes.empty())
    {
        string modePrefix = adding ? "+" : "-";
        string modeMsg = ":" + this->getNickname() + "!" + this->getUsername() + "@localhost MODE " +
                         channelName + " " + modePrefix + appliedModes + appliedArgs + "\r\n";
        channel->broadcast(modeMsg, NULL);
    }
}
