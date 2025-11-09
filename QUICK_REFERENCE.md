# Quick Reference - IRC Operator Commands

## Command Syntax

### KICK
```
KICK <channel> <user> [:<reason>]
```
Ejects a user from a channel (operator only)

**Examples:**
```irc
KICK #general alice :Stop spamming
KICK #test bob
```

---

### INVITE
```
INVITE <nickname> <channel>
```
Invites a user to a channel

**Examples:**
```irc
INVITE alice #private
INVITE bob #members-only
```

---

### TOPIC
```
TOPIC <channel>              (view topic)
TOPIC <channel> :<new topic> (set topic)
```
Views or changes channel topic

**Examples:**
```irc
TOPIC #general
TOPIC #general :Welcome to our channel!
```

---

### MODE
```
MODE <channel>                    (view modes)
MODE <channel> <+/-modes> [args]  (set modes)
```
Views or changes channel modes

**Channel Modes:**
- `+i` / `-i` : Invite-only channel
- `+t` / `-t` : Topic restricted to operators
- `+k <key>` / `-k` : Set/remove channel password
- `+o <nick>` / `-o <nick>` : Give/take operator privilege
- `+l <limit>` / `-l` : Set/remove user limit

**Examples:**
```irc
MODE #test                    # View current modes
MODE #test +i                 # Set invite-only
MODE #test +k secretpass      # Set password
MODE #test +l 50              # Limit to 50 users
MODE #test +o alice           # Make alice operator
MODE #test +itk mypass        # Multiple modes at once
MODE #test -i                 # Remove invite-only
MODE #test +oo alice bob      # Make multiple operators
```

---

## Common Error Codes

| Code | Name | Meaning |
|------|------|---------|
| 401 | ERR_NOSUCHNICK | User doesn't exist |
| 403 | ERR_NOSUCHCHANNEL | Channel doesn't exist |
| 441 | ERR_USERNOTINCHANNEL | User not in that channel |
| 442 | ERR_NOTONCHANNEL | You're not in that channel |
| 443 | ERR_USERONCHANNEL | User already in channel |
| 451 | ERR_NOTREGISTERED | Not registered yet |
| 461 | ERR_NEEDMOREPARAMS | Missing parameters |
| 482 | ERR_CHANOPRIVSNEEDED | Not a channel operator |

---

## Testing Flow

### 1. Start Server
```bash
make
./ircserv 6667 password123
```

### 2. Connect with IRC Client
```bash
nc 127.0.0.1 6667
```

### 3. Register
```irc
PASS password123
NICK alice
USER alice 0 * :Alice Smith
```

### 4. Join Channel (become operator)
```irc
JOIN #test
```

### 5. Test Operator Commands
```irc
# Set channel modes
MODE #test +i
MODE #test +t
MODE #test +k secretkey
MODE #test +l 10

# Change topic (you're operator)
TOPIC #test :New channel topic

# Invite someone
INVITE bob #test

# Give operator to another user
MODE #test +o bob

# Kick someone
KICK #test spammer :No spamming
```

---

## Mode Behavior Summary

| Mode | Effect | Requires Param | Operator Only |
|------|--------|----------------|---------------|
| +i | Invite-only | No | Yes (to set) |
| +t | Topic restricted | No | Yes (to set) |
| +k | Channel password | Yes (key) | Yes (to set) |
| +o | Operator status | Yes (nickname) | Yes (to set) |
| +l | User limit | Yes (number) | Yes (to set) |

---

## Implementation Files

- **`src/Commands.cpp`** - All operator command handlers
  - `handleKick()` - Lines ~90-150
  - `handleInvite()` - Lines ~152-210
  - `handleTopic()` - Lines ~212-270
  - `handleMode()` - Lines ~272-330
  - `applyModeChanges()` - Lines ~345-430

- **`includes/Channel.hpp`** - Channel class with modes
- **`includes/Client.hpp`** - Client class with IRC identity
- **`includes/Utils.hpp`** - IRC numeric reply codes

---

## Permission Requirements

| Command | Requirements |
|---------|--------------|
| KICK | Must be operator in channel |
| INVITE | Must be in channel, operator if +i |
| TOPIC (view) | Must be in channel |
| TOPIC (set) | Must be in channel, operator if +t |
| MODE (view) | None (if channel exists) |
| MODE (set) | Must be operator in channel |

---

## First User = Operator

When a user creates a channel by being the first to JOIN:
- Automatically becomes channel operator
- Can then use all operator commands
- Can grant operator status to others with MODE +o
