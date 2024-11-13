# IRC Server in C++

## Overview

This is a simple Internet Relay Chat (IRC) server implemented in C++. The server allows multiple clients to connect, communicate in real-time, and join different channels. This project serves as a foundational example for understanding network programming and the IRC protocol.

## Features

- **Multi-client Support**: Handles multiple simultaneous connections.
- **Channel Management**: Users can create and join channels.
- **Private Messaging**: Allows users to send direct messages to each other.
- **User Authentication**: Basic user authentication for connecting clients.
- **Command Handling**: Supports standard IRC commands such as JOIN, PART, PRIVMSG, and more.

## Requirements

To build and run this IRC server, you will need:

- A C++ compiler (e.g., g++, clang++)
- Make (for building the project)
- Basic understanding of C++ and network programming concepts

## Installation

1. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/irc-server.git
   cd irc-server
   ```

2. **Build the Project**:
   ```bash
   make
   ```

3. **Run the Server**:
   ```bash
   ./irc_server <port> <password>
   ```

## Usage

Once the server is running, clients can connect using an IRC client (like irssi, HexChat, mIRC, or any other compatible client). 

### Connecting to the Server

1. Open your IRC client.
2. Enter the server address (e.g., `localhost`) and port (default is `6667`).
3. Join a channel using the command:
   ```
   /join #channel_name
   ```

### Commands

Here are some basic commands you can use:

- `/join #channel_name`: Join a specific channel.
- `/part #channel_name`: Leave a channel.
- `/msg username message`: Send a private message to another user.
- `/nick new_nickname`: Change your nickname.

## Video Demonstration

A video demonstration showcasing how to set up and use the IRC server will be available soon. Stay tuned!

## Contact

For questions or feedback, please open an issue in the GitHub repository.

---
