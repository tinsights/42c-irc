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
using std::cout;
using std::endl;

int main(void) {
	int server_socket = socket(AF_INET, SOCK_STREAM, 0);
	
	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET; // ipv4 protocol
	server_addr.sin_port = htons(8080); // host to network byte order (short)
	server_addr.sin_addr.s_addr = INADDR_ANY; // any address is allowed to connect

	bind(server_socket, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));

	listen(server_socket, 5);

	int client_socket = accept(server_socket, NULL, NULL);

	char buffer[1024] = {0};
	int sz = 0;
	do {
		sz = recv(client_socket, buffer, sizeof(buffer), 0);
		cout << "sz: " << sz << " " << buffer;
		memset(buffer, 0, sizeof buffer);
	} while (sz > 0);
	cout << server_socket << endl;
	cout << client_socket << endl;
	close(server_socket);
	close(client_socket);
	
}