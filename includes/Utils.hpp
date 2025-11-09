#pragma once

#include <string>
#include <sstream>

using namespace std;

// IRC Numeric Replies (RFC 1459 & RFC 2812)
namespace RPL {
    // Success replies
    const string RPL_WELCOME = "001";
    const string RPL_NOTOPIC = "331";
    const string RPL_TOPIC = "332";
    const string RPL_INVITING = "341";
    const string RPL_CHANNELMODEIS = "324";
    const string RPL_NAMREPLY = "353";
    const string RPL_ENDOFNAMES = "366";
}

namespace ERR {
    // Error replies
    const string ERR_NOSUCHNICK = "401";
    const string ERR_NOSUCHCHANNEL = "403";
    const string ERR_CANNOTSENDTOCHAN = "404";
    const string ERR_NORECIPIENT = "411";
    const string ERR_NOTEXTTOSEND = "412";
    const string ERR_UNKNOWNCOMMAND = "421";
    const string ERR_NONICKNAMEGIVEN = "431";
    const string ERR_NICKNAMEINUSE = "433";
    const string ERR_USERNOTINCHANNEL = "441";
    const string ERR_NOTONCHANNEL = "442";
    const string ERR_USERONCHANNEL = "443";
    const string ERR_NOTREGISTERED = "451";
    const string ERR_NEEDMOREPARAMS = "461";
    const string ERR_ALREADYREGISTERED = "462";
    const string ERR_PASSWDMISMATCH = "464";
    const string ERR_CHANNELISFULL = "471";
    const string ERR_INVITEONLYCHAN = "473";
    const string ERR_BADCHANNELKEY = "475";
    const string ERR_CHANOPRIVSNEEDED = "482";
}

// Helper function to format IRC numeric reply
inline string formatReply(const string& code, const string& client, const string& message) {
    return ":" + string("server") + " " + code + " " + client + " " + message + "\r\n";
}

// Helper to format numeric reply with channel
inline string formatChannelReply(const string& code, const string& client, const string& channel, const string& message) {
    return ":" + string("server") + " " + code + " " + client + " " + channel + " " + message + "\r\n";
}
