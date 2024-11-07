/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 15:39:27 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/25 13:36:00 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"

int		get_server_socket(const char *port);
void	convertInAddrToString(struct in_addr addr, char *buffer, size_t bufferSize);
void 	execute_cmd(Client &cl, Message & msg);

volatile sig_atomic_t server_running = 1;
void handler(int sig) {
	if (sig == SIGINT) {
		server_running = 0;
	}
}

#define MAX_CONNS 10

bool isAlnumString(string str) {
	for (size_t i = 0; i < str.length(); ++i) {
		if (!std::isalnum(str[i])) {
			return false;
		}
	}
	return true;
}

int main(int ac, char **av) {

	if (ac != 3) {
		std::cerr << "Usage: " << av[0] << " <port> <password>" << std::endl;
		return 1;
	} else if (std::strlen(av[1]) < 4 || std::strlen(av[1]) > 5 || string(av[1]).find_first_not_of("0123456789") != std::string::npos) {
			std::cerr << "Invalid port number. Must be between 1024 and 65535." << std::endl;
			return 1;
	} else {
		int port = std::atoi(av[1]);
		if (port < 1024 || port > 65535) {
			std::cerr << "Invalid port number. Must be between 1024 and 65535." << std::endl;
			return 1;
		}
		string passwd = av[2];
		if (passwd.length() < 3 || !isAlnumString(passwd)) {
			std::cerr << "Invalid password." << std::endl;
			return 1;
		}
		Client::password = passwd;
		Client::port = av[1];
	}

	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);

	/* -------------------------------------------------------------------------- */
	/*                            Server and Poll Init                            */
	/* -------------------------------------------------------------------------- */
	size_t fd_count = 1;
	struct pollfd fds[MAX_CONNS];
	memset(fds, 0, sizeof fds);
	int server_socket = get_server_socket(Client::port.c_str());
	fds[0].fd = server_socket;
	fds[0].events = POLLIN;

	for (size_t i = 1; i < MAX_CONNS; ++i) {
		fds[i].fd = -1; // fd available state
	}

	while (server_running) {
		int poll_count = poll(fds, MAX_CONNS, -1);
		
		for (size_t i = 0; i < MAX_CONNS; ++i) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_socket) {
					/* -------------------------------------------------------------------------- */
					/*                              Accept New Client                             */
					/* -------------------------------------------------------------------------- */

					sockaddr_in remote;
					socklen_t len = sizeof remote;
					int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&remote), &len);
					// use first available pollfd in array:
					for (size_t j = 1; j <= fd_count; ++j) {
						if (fds[j].fd == -1) {
							fds[j].fd = client_socket;
							fds[j].events = POLLIN;
						}
					}
					// Get client IP address
					char ip_str[INET_ADDRSTRLEN];
					convertInAddrToString(remote.sin_addr, ip_str, sizeof(ip_str));

					YEET "client " << fd_count << " at fd " << fds[fd_count].fd << " and ip " << ip_str << " joined!" ENDL;
					Client *client = new Client(client_socket, ip_str);
					Client::connections.insert(std::pair<int, Client &>(client_socket, *client));
					fd_count++;
					YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
				}
				else {

					/* -------------------------------------------------------------------------- */
					/*                     Receive, Parse and Process Message                     */
					/* -------------------------------------------------------------------------- */

					char buffer[1024] = {0};
					int sz = 0;
					sz = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (sz <= 0) {
						// Client quit
						YEET "client " << i << " at fd " <<  fds[i].fd << "quit." ENDL;
						Client & cl = Client::connections.at(fds[i].fd); // may need to guard against SIGINT by checking server_running bool :(
						// dropping connection, check if fully registered in order to remove from "databases"
						if (cl.auth && cl.nick.length()) {
							// remove from all channels
							for (std::set<string>::iterator it = cl.joined_channels.begin(); it != cl.joined_channels.end(); ++it) {
								try {
									Channel & chnl = Channel::channel_list.at(*it);
									chnl.users.erase(cl.nick);
									chnl.opers.erase(cl.nick);
									chnl.invited.erase(cl.nick);
									YEET BOLDBLUE << "Removed " << cl.nick << " from " << *it ENDL;
									YEET BOLDBLUE << "Curr chnl size: " << chnl.users.size() ENDL;
									if (chnl.users.size() == 0) {
										YEET BOLDYELLOW << "Channel " << chnl.name << " is empty. Deleting." ENDL;
										Channel::channel_list.erase(*it);
										delete &chnl;
									}
									else if (chnl.opers.empty()) {
										chnl.opers.insert(*chnl.users.begin());
									}
								}
								catch (std::exception const & e) {
									YEET BOLDRED << "Unable to remove " << cl.nick << " from " << *it ENDL;
								}
							}
							// remove from nickname list
							Client::client_list.erase(cl.nick);
						} else {
							YEET BOLDBLUE << cl.auth << " " << cl.nick ENDL;
						}
				
						close(fds[i].fd);
						delete &cl;
						Client::connections.erase(fds[i].fd);
						fds[i].fd = -1;
						fd_count--;
						YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
					} else {
						// retrieve Client instance
						Client &cl = Client::connections.at(fds[i].fd);
						YEET "\nRECVD FROM CLIENT " << i << " at fd " << fds[i].fd  << "\n" << BOLDBLUE << buffer ENDL;
						// parse message into command
						string message(buffer);
						// reconstruct from remainder if present
						if (cl.remainder.length()) {
							YEET BOLDYELLOW << "Inserting Remainder: "	<< cl.remainder ENDL;
							message.insert(0, cl.remainder);
						}
						// check if message has CRLF
						if (!message.empty()) {
							size_t idx = message.find("\r\n", 0);
							// potentially multiple CRLF commands in one "transmission"
							while (idx != string::npos) {
								string cmd = message.substr(0, idx);
								/* -------------------------------------------------------------------------- */
								/*                               EXECUTE COMMAND                              */
								/* -------------------------------------------------------------------------- */
								Message msg(cmd);
								if (msg.valid) {
									execute_cmd(cl, msg);
								}
								message.erase(0, idx + 2); // +2 bc of crlf
								idx = message.find("\r\n", 0);
							}
							// store remainder if present.
							if (!message.empty()) {
								cl.remainder = message;
								YEET BOLDYELLOW << "REMAINDER: " << cl.remainder ENDL;
							} else if (cl.remainder.length()) {
								cl.remainder.clear();
							}
						}
					}
					memset(buffer, 0, sizeof buffer);
				}
			}
		}
		if (poll_count == 0) // maybe unnec
			break;
	}

	YEET "EXITING" ENDL;
	for (std::map<int, Client &>::iterator it = Client::connections.begin(); it != Client::connections.end(); ++it) {
		delete &it->second;
		close(it->first);
	}
	close(server_socket);
	for (std::map<string, Channel&>::iterator it = Channel::channel_list.begin(); it != Channel::channel_list.end(); ++it) {
		delete &it->second;
	}
}
