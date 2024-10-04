/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/25 15:39:27 by tjegades          #+#    #+#             */
/*   Updated: 2024/09/25 15:39:28 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"
#include <csignal>
#define MAX_CONNS 10

int		get_server_socket();
void	convertInAddrToString(struct in_addr addr, char *buffer, size_t bufferSize);

volatile sig_atomic_t server_running = 1;


void handler(int sig) {
	if (sig == SIGINT) {
		server_running = false;
	}
}

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

	while (server_running) {
		int poll_count = poll(fds, fd_count, -1);
		
		for (size_t i = 0; i < fd_count; ++i) {
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
					fds[fd_count].fd = client_socket;
					fds[fd_count].events = POLLIN;
					cout << "client " << fd_count << " at fd " << fds[fd_count].fd << " joined!" << endl;

					// Get client IP address
					char ip_str[INET_ADDRSTRLEN];
					convertInAddrToString(remote.sin_addr, ip_str, sizeof(ip_str));
					cout << ip_str << endl;
					fd_count++;
				}
				else {
					char buffer[1024] = {0};
					int sz = 0;
					sz = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					if (sz <= 0) {
						cout << "client " << i << " at fd " <<  fds[i].fd << "quit." << endl;
						close(fds[i].fd);
						fd_count--;
					} else {
						// broadcast to other clients
						cout << "received " << buffer << " from client " << i << " at fd " << fds[i].fd << endl;
						for (size_t j = 1; j < fd_count; ++j) {
							if (fds[j].fd != fds[i].fd)
								send(fds[j].fd, buffer, sizeof buffer, 0);
						}
					}
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
