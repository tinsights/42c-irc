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

#include <sys/socket.h> // cpp ver?
#include <netdb.h> // cpp ver?
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <poll.h>

using std::cout;
using std::endl;

typedef struct addrinfo ai;

int main(void) {

	/**
	 * //OLD WAY! :
	 * sockaddr_in server_addr = {
	 * 	.sin_family = AF_INET,		// Address Family IPV4
	 * 	.sin_port = htons(8080),	// Host to network byte order (short) 8080
	 * 	.sin_addr = { INADDR_ANY }, // Accept from any address
	 * };
	 * 
	 * new way of getting config options:
	*/
	ai hints, *res;

	memset(&hints, 0, sizeof hints);

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, "8080", &hints, &res);
	
	/**
	 * #include <sys/types.h>
	 * #include <sys/socket.h>
	 * int socket(int domain, int type, int protocol);
	 * 
	 * socket() returns a new file descriptor configured to act as a socket
	 * with the specified domain (Protocol Family), type (datagram or TCP),
	 * and protocol (getprotobyname())
	 * 
	 * TODO: error checking (i.e. check return value)
	*/
	int server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	/**
	 * int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
	 * 
	 * bind() associates a socket with an IP address and a port number.
	 * Prior to this a socket is just a file / file descriptor.
	 * this socket listens to connections.
	 * 
	 * TODO: error checking (i.e. check return value)
	*/
	bind(server_socket, res->ai_addr, res->ai_addrlen);

	/**
	 * int listen(int s, int backlog);
	 * 
	 * This instructs the socket to listen for incoming connections from client programs:
	 * The second argument is the number of incoming connections
	 * that can be queued before you call accept(), below.
	 * 
	 * If there are this many connections waiting to be accepted,
	 * additional clients will generate the error ECONNREFUSED.
	 * 
	 * TODO: error checking (i.e. check return value)
	*/
	listen(server_socket, 10);

	/**
	 * int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
	 * 
	 * accept() returns another socket file descriptor, one that is connected to the client.
	 * it fills sockaddr *addr with information about the connecting client, 
	 * and returns a new socket FD that can be used for send() and recv()
	 * 
	 * TODO: error checking (i.e. check return value)
	*/
	// sockaddr_storage remote;
	// socklen_t len = sizeof remote;
	// int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&remote), &len);

	size_t fd_max = 10;
	size_t fd_count = 1;
	struct pollfd fds[fd_max];
	fds[0].fd = server_socket;
	fds[0].events = POLLIN;

	for (;;) {
		int poll_count = poll(fds, fd_count, -1);
		
		for (size_t i = 0; i < fd_max; ++i) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == server_socket) {
					sockaddr_storage remote;
					socklen_t len = sizeof remote;
					int client_socket = accept(server_socket, reinterpret_cast<sockaddr *>(&remote), &len);
					fds[fd_count].fd = client_socket;
					fds[fd_count].events = POLLIN;
					fd_count++;
				}
				else {
					char buffer[1024] = {0};
					int sz = 0;
					sz = recv(fds[i].fd, buffer, sizeof(buffer), 0);
					// cout << "sz: " << sz << " " << buffer;
					if (sz <= 0) {
						close(fds[i].fd);
						fd_count--;
					} else {
						for (size_t j = 1; j < fd_count; ++j) {
							if (j != i)
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

	// cout << server_socket << endl;
	// cout << client_socket << endl;
	close(server_socket);
	// close(client_socket);
}
