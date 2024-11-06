/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   serverutils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 11:56:26 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/03 11:56:27 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

#define MAX_QUEUE 10

int get_server_socket(const char * port) {
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
	int server_socket;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, port, &hints, &res);
	
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
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_socket == -1) {
		perror("socket");
		exit(1);
	}

	int optval = 1;
	if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval)) {
		perror("setsockopt");
		exit(1);
	}
	/**
	 * int bind(int sockfd, struct sockaddr *my_addr, socklen_t addrlen);
	 * 
	 * bind() associates a socket with an IP address and a port number.
	 * Prior to this a socket is just a file / file descriptor.
	 * this socket listens to connections.
	 * 
	 * TODO: error checking (i.e. check return value)
	*/
	if (bind(server_socket, res->ai_addr, res->ai_addrlen)) {
		perror("bind");
		exit(1);
	}

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
	if (listen(server_socket, MAX_QUEUE)) {
		perror("listen");
		exit(1);
	}

	freeaddrinfo(res);
	return server_socket;
}

void convertInAddrToString(struct in_addr addr, char *buffer, size_t bufferSize) {
	snprintf(buffer, bufferSize, "%u.%u.%u.%u", 
		(addr.s_addr & 0xFF), 
		(addr.s_addr >> 8) & 0xFF, 
		(addr.s_addr >> 16) & 0xFF, 
		(addr.s_addr >> 24) & 0xFF);
}
