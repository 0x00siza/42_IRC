Excellent! Having your core classes (`Server`, `Client`, `Channel`) and their member variables outlined is a huge step. Now you're ready to start implementing the server's operational logic.

The next logical steps focus on **setting up the basic server infrastructure** to accept connections and manage I/O, as dictated by the project's mandatory part.

Here's a breakdown of what to do next:

### Phase 1: Basic Server Setup & Connection Handling

1.  **Implement Server Constructor & Socket Initialization:**
    *   In your `Server` class constructor, use the `_port` member to create and configure the listening socket.
    *   **Steps:**
        *   `socket()`: Create the listening socket. Store its file descriptor in `_listeningSocketFd`.
        *   `setsockopt()`: Set socket options, most importantly `SO_REUSEADDR` to avoid "Address already in use" errors when restarting your server quickly.
        *   `bind()`: Associate the socket with your server's IP address (usually `INADDR_ANY` for all local interfaces) and `_port`.
        *   `listen()`: Put the socket into listening mode, ready to accept incoming connections.
        *   **Error Handling:** Crucially, check the return values of *all* these functions. If any fail, print an error to `std::cerr` and terminate the server gracefully (or throw an exception if you decide to handle errors that way).

2.  **Make the Listening Socket Non-Blocking:**
    *   Your project requires all I/O to be non-blocking.
    *   Use `fcntl(sockfd, F_SETFL, O_NONBLOCK);` for your `_listeningSocketFd`. Remember the MacOS-specific rule for `fcntl()`.
    *   **Error Handling:** Check `fcntl()`'s return value.

3.  **Initialize `_pollFds` with the Listening Socket:**
    *   Create a `struct pollfd` for your `_listeningSocketFd`.
    *   Set its `fd` to `_listeningSocketFd`.
    *   Set its `events` to `POLLIN` (to monitor for incoming connection requests).
    *   Add this `pollfd` structure to your `_pollFds` vector.

4.  **Implement the Main Server Loop (`run()` method):**
    *   Create a `public` method in your `Server` class, perhaps named `run()`, which will contain the server's main event loop.
    *   This loop will continuously call `poll()`.
    *   **Inside the loop:**
        *   Call `poll(&_pollFds[0], _pollFds.size(), -1);` (The `-1` timeout makes `poll` block indefinitely until an event occurs).
        *   **Error Handling:** Check `poll()`'s return value. A return of `-1` indicates an error (and `errno` will be set).
        *   Iterate through `_pollFds` to check for events.

5.  **Handle New Client Connections:**
    *   Inside the main loop, if `poll()` indicates an event on `_listeningSocketFd` (`_pollFds[0].revents & POLLIN`):
        *   **`accept()`:** Accept the new connection. This returns a *new* socket file descriptor for the client.
        *   **Error Handling:** Check `accept()`'s return value. If it's `-1`, print an error. Since it's non-blocking, you might get `EAGAIN` or `EWOULDBLOCK`, which means no client is waiting (you can ignore this and continue the loop, though `POLLIN` should prevent it usually).
        *   **Make Client Socket Non-Blocking:** Apply `fcntl(client_fd, F_SETFL, O_NONBLOCK);` to this *new* client socket.
        *   **Create `Client` Object:** Instantiate a new `Client` object, passing its `socketFd` to its constructor.
        *   **Store Client:** Add this `Client` object (or a pointer to it) to your `_clients` map, using its `socketFd` as the key.
        *   **Add to `_pollFds`:** Create a new `struct pollfd` for this client's socket (`client_fd`), set its `events` to `POLLIN | POLLOUT` (you'll want to read and potentially write to it), and add it to your `_pollFds` vector.

6.  **Handle Client Disconnections:**
    *   When `poll()` indicates an event on a client socket where `revents & (POLLERR | POLLHUP | POLLNVAL)` (error, hang up, invalid request) or `recv()` returns 0 bytes (graceful disconnect):
        *   **Clean Up:**
            *   Close the client's socket: `close(client_fd);`
            *   Remove the client's `pollfd` from your `_pollFds` vector. Be careful here, as removing an element can shift indices. A common pattern is to remove it and then decrement your loop counter if iterating.
            *   Delete the `Client` object from your `_clients` map (and `delete` the `Client` object if using `new`).
            *   Log the disconnection.

### Phase 2: Basic Client I/O (Read and Write)

7.  **Handle Incoming Client Data (`POLLIN`):**
    *   Inside the main loop, for each client `pollfd` where `revents & POLLIN`:
        *   **`recv()`:** Call `recv(client_fd, buffer, buffer_size, 0);`
        *   **Error Handling:**
            *   If `recv()` returns `0`, it's a disconnect (handle as above).
            *   If `recv()` returns `-1` and `errno == EAGAIN` or `EWOULDBLOCK`, there's no data currently available (normal for non-blocking; ignore and continue).
            *   If `recv()` returns `-1` for other `errno` values, it's a read error (treat as disconnect or log error).
        *   **Append to `_readBuffer`:** If data is received, append it to the `_readBuffer` of the corresponding `Client` object.
        *   **Process Commands:** Check the `Client`'s `_readBuffer` for complete IRC messages (usually ending with `\r\n`). When a complete message is found:
            *   Extract the complete message from the buffer.
            *   **Parse the IRC command.** (This will be a significant next step on its own).
            *   Execute the command's logic.
            *   Remove the processed message from `_readBuffer`.

8.  **Handle Outgoing Client Data (`POLLOUT`):**
    *   Inside the main loop, for each client `pollfd` where `revents & POLLOUT`:
        *   If the `Client` object's `_writeBuffer` is *not empty*:
            *   **`send()`:** Call `send(client_fd, _writeBuffer.c_str(), _writeBuffer.length(), 0);`
            *   **Error Handling:** Check `send()`'s return value. If it's `-1` and `errno == EAGAIN` or `EWOULDBLOCK`, not all data could be sent; you'll need to store how much was sent and try sending the rest later. Other errors mean a write error (treat as disconnect).
            *   **Update `_writeBuffer`:** If `send()` was successful, remove the sent data from the beginning of `_writeBuffer`.
            *   **Disable `POLLOUT` if buffer is empty:** If `_writeBuffer` becomes empty after sending, you can modify the client's `pollfd.events` to remove `POLLOUT` (you only need `POLLOUT` if there's data waiting to be sent). Re-enable it when you add data to `_writeBuffer`. This prevents `poll()` from continuously reporting `POLLOUT` readiness when there's nothing to send.

### Initial Test Cycle:

At this stage, you should be able to:

1.  Run your `ircserv`.
2.  Connect to it with `irssi` (`/connect 127.0.0.1 <port> <password>`).
3.  Your server should detect the new connection, create a `Client` object, and add its socket to `_pollFds`.
4.  If you type something in `irssi` (even before registering), your server should `recv()` that data and append it to the `Client`'s `_readBuffer`. You can print `_readBuffer` to `std::cout` on your server's terminal to verify this.

This is a lot, but it builds up piece by piece. Start with the socket initialization, then the `poll()` loop, then handling new connections, and finally basic read/write for clients.

`
`
This visual depicts the core components and their interaction: the main server loop orchestrating `poll()` to monitor all client connections (and the listening socket), routing data to and from individual `Client` objects, and managing the `Channel` objects.