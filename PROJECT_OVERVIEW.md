# 42 IRC Server Project

## What is this project?

This is a **custom IRC (Internet Relay Chat) server** built from scratch in **C++98**. Think of it like creating your own Discord or Slack server, but using the older IRC protocol that's been around since the late 1980s.

## What does it do?

Our IRC server allows multiple people to:
- **Connect** from different computers using IRC clients (like HexChat, irssi, etc.)
- **Chat** with each other in real-time
- **Join channels** (like chat rooms) to talk with multiple people
- **Send private messages** to specific users
- **Manage channels** with operators who can kick/ban users

## Why build this?

This project is part of the **42 School curriculum**. It teaches us:
- **Network programming** - How computers communicate over the internet
- **Socket programming** - Low-level network communication in C++
- **I/O multiplexing** - Handling many clients simultaneously without blocking
- **Protocol implementation** - Following the IRC RFC standards
- **Server architecture** - Designing scalable, reliable server software

## How does it work? (Simple explanation)

### 1. **Starting the Server**
```bash
./ircserv 6667 mypassword
```
- The server listens on port 6667 (standard IRC port)
- Requires a password for security

### 2. **Client Connection Process**
```
1. Client connects to server → Server accepts connection
2. Client sends password → Server verifies it
3. Client sets nickname → Server checks if it's unique  
4. Client sends user info → Server completes registration
5. Client is now ready to chat!
```

### 3. **What happens behind the scenes**
- Server uses **sockets** to communicate with clients
- **poll()** system call monitors all clients simultaneously
- **Non-blocking I/O** ensures server stays responsive
- **Command parsing** interprets IRC protocol messages
- **State management** tracks who's connected, in which channels, etc.

## Technical Architecture

### Core Components

#### 🖥️ **Server Class**
- Manages the main listening socket
- Handles new client connections
- Coordinates all server operations
- Uses `poll()` for efficient I/O multiplexing

#### 👤 **Client Class**  
- Represents each connected user
- Manages client state (nickname, authentication, etc.)
- Handles incoming/outgoing messages for that user
- Tracks registration status

#### 🏠 **Channel Class** 
- Represents chat rooms (#general, #random, etc.)
- Manages who's in each channel
- Handles channel-specific permissions and modes

#### 📝 **Command Class**
- Parses IRC protocol messages
- Executes different IRC commands (NICK, JOIN, PRIVMSG, etc.)
- Handles command validation and error responses

### Key Technologies Used

| Technology | Purpose | Why it matters |
|------------|---------|----------------|
| **C++98** | Programming language | Legacy compatibility, manual memory management |
| **Berkeley Sockets** | Network communication | Standard Unix networking API |
| **poll()** | I/O multiplexing | Handle many clients efficiently |
| **Non-blocking I/O** | Concurrent connections | Server doesn't freeze waiting for slow clients |
| **IRC Protocol (RFC 1459/2812)** | Communication standard | Compatibility with existing IRC clients |

## Project Structure

```
📁 irc/
├── 📄 Makefile           # Build configuration
├── 📄 README.md          # Testing guide with HexChat
├── 📄 PROJECT_OVERVIEW.md # This file - project explanation
├── 📁 includes/          # Header files
│   ├── Server.hpp        # Server class definition
│   ├── Client.hpp        # Client class definition  
│   ├── Command.hpp       # Command parsing
│   ├── Channel.hpp       # Channel management (future)
│   └── Utils.hpp         # Utility functions
└── 📁 src/               # Source code
    ├── main.cpp          # Program entry point
    ├── Server.cpp        # Server implementation
    ├── Client.cpp        # Client handling
    ├── Command.cpp       # IRC commands (PASS, NICK, USER, etc.)
    ├── Channel.cpp       # Channel operations (future)
    └── Utils.cpp         # Helper functions
```

## IRC Protocol Basics

### What is IRC?
IRC (Internet Relay Chat) is a text-based communication protocol. Users connect to IRC servers and can:
- Join channels (chat rooms that start with #)
- Send private messages to other users
- Share files and links
- Use various commands to manage their experience

### Basic IRC Commands We Implement

| Command | Purpose | Example |
|---------|---------|---------|
| `PASS` | Authenticate with server | `PASS secretpassword` |
| `NICK` | Set your nickname | `NICK alice` |
| `USER` | Set user information | `USER alice 0 * :Alice Smith` |
| `JOIN` | Join a channel | `JOIN #general` |
| `PRIVMSG` | Send message | `PRIVMSG #general :Hello everyone!` |
| `QUIT` | Disconnect | `QUIT :Goodbye!` |

### Message Format
IRC messages follow this structure:
```
[:<prefix>] <command> [<params>] [:<trailing>]\r\n
```

**Example:**
```
:alice!user@host PRIVMSG #general :Hello everyone!
```

## Development Phases

### ✅ **Phase 1: Foundation** 
- ✅ TCP server with socket programming
- ✅ Client connection handling
- ✅ Basic command parsing
- ✅ Registration system (PASS/NICK/USER)
- ✅ Error handling with IRC numeric codes

### 🚧 **Phase 2: Core Features**
- 🔄 Channel management (JOIN/PART)
- 🔄 Private messaging (PRIVMSG)
- 🔄 Channel operators and modes
- 🔄 User management (KICK/BAN)

### 📋 **Phase 3: Bonus Features** 
- 📋 File transfers
- 📋 A bot

## How to test it?

### Quick Start
1. **Build:** `make`
2. **Run:** `./ircserv 6667 mypassword`  
3. **Connect:** Use any IRC client (HexChat, irssi, etc.) to connect to `localhost:6667`

### 📋 **Detailed Testing Guide**
For comprehensive testing instructions
**👉 check the complete testing guide: [README.md](README.md)**


## Real-World Applications

The skills learned building this IRC server apply to:
- **Web servers** (handling HTTP requests)
- **Game servers** (real-time multiplayer games)  
- **Chat applications** (Discord, Slack, WhatsApp backends)
- **IoT systems** (device communication protocols)
- **Microservices** (inter-service communication)
- **Any networked application** requiring real-time communication

## Resources and References

### IRC Protocol Documentation
- [RFC 1459](https://tools.ietf.org/html/rfc1459) - Original IRC Protocol
- [RFC 2812](https://tools.ietf.org/html/rfc2812) - Updated IRC Client Protocol

---

## Contributors

This project is developed by **42 School students** as part of the curriculum.

**Happy coding! 💜**

*"The best way to learn networking is to build something that networks."*