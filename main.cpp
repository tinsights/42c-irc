/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 15:39:27 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/20 14:35:59 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"

int		get_server_socket();
void	convertInAddrToString(struct in_addr addr, char *buffer, size_t bufferSize);
void 	execute_cmd(Client &cl, string &cmd);

/**
 * seems insufficient for clean exit,
 * occassionally ctrl+c gives memleaks.. investigate later
*/

volatile sig_atomic_t server_running = 1;
void handler(int sig) {
	if (sig == SIGINT) {
		server_running = 0;
	}
}

/**
 * Currently "global" cause idk who needs what
 * Eventually in either .cpp as static? or just regular private attr
 * 
 * or in Repo Class managing all data and events => atomic state changes.
 * https://www.geeksforgeeks.org/repository-design-pattern/
 * */ 
std::map<string, Client &> Client::client_list;
std::map<string, Channel & > Channel::channel_list; // change to channel name and channel instance
std::map<int, Client> connections; // global for now.

#define MAX_CONNS 100

int main(void) {

	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);

	size_t fd_count = 1;
	struct pollfd fds[MAX_CONNS];
	memset(fds, 0, sizeof fds);
	int server_socket = get_server_socket();
	fds[0].fd = server_socket;
	fds[0].events = POLLIN;

	for (size_t i = 1; i < MAX_CONNS; ++i) {
		fds[i].fd = -1; // fd available state
	}


	while (server_running) {
		int poll_count = poll(fds, MAX_CONNS, -1); // changed to poll over whole range, as opposed to till fd_count, for now.
		
		for (size_t i = 0; i < MAX_CONNS; ++i) { // changed to poll over whole range, for now.
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_socket) {
					// received new client connection,
					// to be added to fds array
					/**
					 * int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
					 * 
					 * accept() returns another socket file descriptor, one that is connected to the client.
					 * it fills sockaddr *addr with information about the connecting client, 
					 * and returns a new socket FD that can be used for send() and recv()
					 * 
					 * TODO: error checking (i.e. check return value)
					*/
					sockaddr_in remote;
					socklen_t len = sizeof remote;
					int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&remote), &len);
					// use first available pollfd in array:
					for (size_t j = 1; j <= fd_count; ++j) {
						if (fds[j].fd == -1) {
							fds[j].fd = client_socket; // <--- culprit of random seggies. previously was fds[fd_count].
							fds[j].events = POLLIN;
						}
					}
					// Get client IP address
					// for fun tbh. this was AI gen code.
					char ip_str[INET_ADDRSTRLEN];
					convertInAddrToString(remote.sin_addr, ip_str, sizeof(ip_str));

					YEET "client " << fd_count << " at fd " << fds[fd_count].fd << " and ip " << ip_str << " joined!" ENDL;
					Client client(client_socket, ip_str);
					connections.insert(std::pair<int, Client &>(client_socket, client));
					fd_count++;
					YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
				}
				else {
					char buffer[1024] = {0};
					int sz = 0;
					sz = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (sz <= 0) {
						YEET "client " << i << " at fd " <<  fds[i].fd << "quit." ENDL;
						/**
						 * MARK: SEGGY IF CLIENT CLOSES TERMINAL!
						 * FIXED!
						*/
						Client cl = connections.at(fds[i].fd); // need to guard against SIGINT by checking server_running bool :(
						// dropping connection, check if fully registered in order to remove from "databases"
						if (cl.auth && cl.nick.length()) {
							// remove from all channels
							for (std::set<string>::iterator it = cl.joined_channels.begin(); it != cl.joined_channels.end(); ++it) {
								try {
									Channel::channel_list.at(*it).users.erase(cl.nick);
									YEET BOLDBLUE << "Removed " << cl.nick << " from " << *it ENDL;
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
						connections.erase(fds[i].fd);
						fds[i].fd = -1; // setting to -1 as a signal that it can be re-used
						fd_count--;		// do i need fd count anymore? idk
						YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
					} else {
						// retrieve Client instance
						Client &cl = connections.at(fds[i].fd);
						
						YEET "\nRECVD FROM CLIENT " << i << " at fd " << fds[i].fd  << "\n" << BOLDBLUE << buffer ENDL;
						
						// parse message into command
						string message(buffer);
						// reconstruct from remainder if present
						if (cl.remainder.length()) {
							message.insert(0, cl.remainder);
						}
						// check if message has CRLF
						if (!message.empty()) {
							size_t idx = message.find("\r\n", 0);

							// potentially multiple CRLF commands in one "transmission"
							while (idx != string::npos) {
								string cmd = message.substr(0, idx);
								execute_cmd(cl, cmd);
								message.erase(0, idx + 2); // +2 bc of crlf
								idx = message.find("\r\n", 0);
							}
							// store remainder if present.
							if (!message.empty()) {
								cl.remainder = message;
							} else if (cl.remainder.length()) {
								cl.remainder.clear();
							}
						}
					}
					// reset
					memset(buffer, 0, sizeof buffer);
				}
			}
		}
		if (poll_count == 0) // maybe unnec
			break;
	}

	cout << server_socket << endl;
	cout << fd_count << endl;
	cout << "EXITING" << endl;
	
	for (size_t i = 0; i < fd_count; ++i)
		close(fds[i].fd);
}
