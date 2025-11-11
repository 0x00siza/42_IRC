# IRC Server - Testing Guide with HexChat

## Overview
This is a custom IRC server implementation written in C++98. This guide will help you test the server functionality using HexChat as the IRC client.

## Building and Running the Server

### Prerequisites
- C++ compiler with C++98 support
- Linux/macOS/Unix system
- HexChat IRC client

### Build Instructions
```bash
# Clone and navigate to the project directory
cd /path/to/irc

# Build the server
make

# Run the server
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mypassword
```

## Installing and Setting Up HexChat

### Installation
```bash
# Debian/Ubuntu
sudo apt update && sudo apt install hexchat

# macOS (with Homebrew)
brew install --cask hexchat
```

### Launch HexChat
```bash
hexchat &
```

## Testing the IRC Server

### 1. Basic Connection Setup

#### Method 1: Quick Connect
1. Start HexChat
2. Go to **HexChat** → **Network List**
3. Click **Add** and name it "Local IRC Server"
4. Select your new network and click **Edit**
5. In the **Servers** tab, add: `localhost/6667` (or your chosen port)
6. **Uncheck** "Use SSL for all servers on this network"
7. Set your **Nick name** (e.g., "testuser")
8. Click **Connect**

#### Method 2: Manual Connection
1. Start HexChat
2. In any window, type:
```
/server localhost 6667
```

### 2. Testing Registration Commands

Our IRC server requires a 3-step registration process. Test in this exact order:

#### Step 1: PASS Command
```
/quote PASS mypassword
```
**Expected behavior:**
- ✅ Server accepts password if correct
- ❌ Server sends error 464 "Password Incorrect" if wrong
- ❌ Server sends error 462 "You may not reregister" if already registered

#### Step 2: NICK Command  
```
/nick testnick
```
**Expected behavior:**
- ✅ Sets nickname if unique and valid
- ❌ Error 431 "No nickname given" if empty
- ❌ Error 432 "Erroneous nickname" if invalid characters
- ❌ Error 433 "Nickname is already in use" if taken

**Invalid nickname examples to test:**
```
/nick #badnick     # starts with #
/nick :badnick     # starts with :
/nick bad nick     # contains space
/nick verylongnickname  # over 9 characters
```

#### Step 3: USER Command
```
/quote USER testuser 0 * :Real Name Here
```
**Expected behavior:**
- ✅ Completes registration if PASS and NICK already set
- ❌ Error 461 "Not enough parameters" if missing parameters
- ❌ Error 462 "You may not reregister" if already set

**Complete registration sequence example:**
```
/quote PASS mypassword
/nick alice
/quote USER alice 0 * :Alice Smith
```

### 3. Testing Post-Registration Features

#### Change Nickname (after registration)
```
/nick newnickname
```

#### JOIN Command (if implemented)
```
/join #testchannel
```

#### QUIT Command
```
/quit
```
or
```
/quote QUIT :Goodbye!
```

### 4. Testing Error Conditions

#### Authentication Errors
```bash
# Test wrong password
/quote PASS wrongpassword
/nick testuser
/quote USER testuser 0 * :Test User
# Should get "Password Incorrect" error
```

#### Registration Before Authentication
```bash
# Try commands without PASS first
/nick testuser     # Should get "You have not registered"
/join #test        # Should get "You have not registered"
```

#### Duplicate Registration
```bash
# After successful registration, try again:
/quote PASS mypassword  # Should get "You may not reregister"
/quote USER newuser 0 * :New User  # Should get "You may not reregister"
```

#### Nickname Conflicts
```bash
# Connect with two different HexChat instances
# Client 1: /nick alice
# Client 2: /nick alice  # Should get "Nickname is already in use"
```

### 5. Advanced Testing Scenarios

#### Multiple Clients
1. Open multiple HexChat windows or instances
2. Connect each to your server with different nicknames
3. Test nickname uniqueness
4. Test server capacity

#### Connection Limits
- Connect many clients to test the server's handling of multiple connections
- Test graceful disconnection handling

#### Buffer Testing
- Send very long messages (test 510 character limit)
- Send rapid messages to test buffer handling
- Test incomplete messages (without \r\n)

#### Malformed Commands
```
/quote INVALIDCOMMAND
/quote NICK
/quote USER insufficient params
```

## Debugging Tips

### Server-Side Debugging
Watch your server terminal for output. The server should show:
```
Server is running on 0.0.0.0:6667 with (fd=3)
waiting for connections...
accepted 127.0.0.1:54321 (fd=4)
Client 4 changed nick to: testnick
Client testnick is fully registered!
```

### HexChat Debugging
1. **Enable Raw Log**: Go to **View** → **Raw Log** to see all IRC protocol messages
2. **Server Messages**: Watch the server tab for error messages and replies
3. **Status Window**: Check the status window for connection information

### Common Issues and Solutions

| Issue | Cause | Solution |
|-------|--------|----------|
| "Connection refused" | Server not running | Start server with `./ircserv 6667 mypassword` |
| "Password Incorrect" | Wrong password | Use correct password in PASS command |
| "You have not registered" | Missing registration steps | Complete PASS → NICK → USER sequence |
| "Nickname is already in use" | Duplicate nickname | Choose a different nickname |
| No response to commands | Wrong command format | Check IRC command syntax |

## Testing Checklist

### Basic Functionality
- [ ] Server starts without errors
- [ ] HexChat can connect to server
- [ ] PASS command works with correct password
- [ ] PASS command rejects incorrect password
- [ ] NICK command sets valid nicknames
- [ ] NICK command rejects invalid nicknames
- [ ] USER command completes registration
- [ ] Server tracks registered clients
- [ ] Client can change nickname after registration
- [ ] QUIT command disconnects client properly

### Error Handling
- [ ] Registration commands in wrong order
- [ ] Duplicate registration attempts  
- [ ] Invalid command parameters
- [ ] Commands before authentication
- [ ] Nickname conflicts between clients
- [ ] Connection drops handled gracefully

### Multi-Client Testing
- [ ] Multiple simultaneous connections
- [ ] Unique nickname enforcement
- [ ] Client list management
- [ ] Proper cleanup on disconnect

## IRC Protocol References

For understanding IRC protocol details:
- [RFC 1459](https://tools.ietf.org/html/rfc1459) - Internet Relay Chat Protocol
- [RFC 2812](https://tools.ietf.org/html/rfc2812) - Internet Relay Chat: Client Protocol

## Project Structure
```
├── Makefile           # Build configuration
├── src/
│   ├── main.cpp       # Server entry point  
│   ├── Server.cpp     # Core server implementation
│   ├── Client.cpp     # Client connection handling
│   ├── Command.cpp    # IRC command implementations
│   └── Channel.cpp    # Channel management (future)
└── includes/          # Header files
```

## Current Implementation Status

### ✅ Implemented Features
- TCP socket server with poll() for I/O multiplexing
- Non-blocking client connections
- IRC message parsing (prefix, command, parameters)
- Registration system (PASS/NICK/USER)
- Basic error responses with proper IRC numeric codes
- Nickname uniqueness validation
- Client authentication and registration tracking

### 🚧 In Development  
- JOIN/PART channel commands
- PRIVMSG (private messages)
- Channel management
- More IRC commands (KICK, MODE, etc.)

### 📋 Planned Features
- Channel operators and modes
- Server-to-server communication
- Advanced IRC features

---

**Happy Testing! 💜**
