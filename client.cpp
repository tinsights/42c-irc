/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 11:56:58 by tjegades          #+#    #+#             */
/*   Updated: 2024/09/30 11:56:59 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cerrno>

using std::cout;
using std::endl;
using std::string;

int main (int ac, char **av) {
	string msg = "";

	if (ac == 2)
		msg = av[1];
	
	cout << msg << endl;
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(8080),
		.sin_addr = { INADDR_ANY },
	};

	bind(server_socket,reinterpret_cast<struct sockaddr*>(&server_addr), sizeof server_addr);
	connect(server_socket,reinterpret_cast<struct sockaddr*>(&server_addr), sizeof server_addr);

	if (ac == 2)
		send(server_socket, msg.c_str(), msg.length(), 0);
	else {
		std::string buffer;

		ssize_t sz;
		
		do {
			std::getline(std::cin, buffer);
			sz = send(server_socket, buffer.c_str(), buffer.length(), 0);
			cout << "sz: " << sz << endl;
		} while (sz > 0);
		std::perror("error");
		cout << server_socket << endl;
	}
	close(server_socket);
}