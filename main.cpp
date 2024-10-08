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
#include <map>
#include <algorithm>

#define MAX_CONNS 10

int		get_server_socket();
void	convertInAddrToString(struct in_addr addr, char *buffer, size_t bufferSize);

volatile sig_atomic_t server_running = 1;


void handler(int sig) {
	if (sig == SIGINT) {
		server_running = false;
	}
}

class Client {
	public:
		Client(int fd, string ip) { socket = fd; ip_addr = ip; std::clog << ip << "  " << ip_addr.c_str() << "  " << ip_addr.length() << endl;} ;
		// TODO: copy
		~Client() {};
		int 	socket;
		string	ip_addr;
		string	nick;
		string	user;
		string	host;
		string	fullname;
		string	remainder;
		bool	auth;
		bool	registered;

	private:
		Client() {};
		
};

class Message {
	public:
	Message(string raw) {
		/**
		 * TODO: 
		 * 	sanity checks (length, not empty, etc.)
		 * 	ends with CRLF?
		 * 	total validity check? max 2 colons? num spaces? etc
		 * 
		 * */ 
		if (raw.find_first_of(':') == 0) {
			prefix = raw.substr(0, raw.find_first_of(' '));
			raw.erase(0, raw.find_first_of(' ') + 1);
		} else {
			prefix = "";
		}
		cmd = raw.substr(0, raw.find_first_of(' '));
		raw.erase(0, raw.find_first_of(' ') + 1);
		
		if (raw.length()) {
			size_t trail_index = raw.find_first_of(':');
			params = raw.substr(0, trail_index);
			if (trail_index != string::npos) {
				trailing = raw.substr(trail_index + 1);
			}
		}
		cout << "Prefix: " << prefix << "\nCmd: " << cmd << "\nParams: " << params << "\nTrail: " << trailing << endl;
	};
	~Message() {};

	// eventually private.. i think?
	string	input;
	string	cmd;
	string	prefix;
	string	trailing;
	string	params;
	bool	valid;
	private:

};

#define NUM_CMDS 4
void execute_cmd(Client &cl, string &cmd) {
	Message msg(cmd);
	string cmds[NUM_CMDS] = {"PASS", "NICK", "USER", "PRIVMSG"};
	int index = -1;
	for (int i = 0; i < NUM_CMDS; ++i) {
		if (msg.cmd == cmds[i]) {
			// cout << cmds[i] << endl;
			// string response = "Received: ";
			// response.append(cmd);
			// send(cl.socket, response.c_str(), response.length(), 0);
			index = i;
			break;
		}
	}
	// upstream msg validity check??
	string response = "";
	switch (index) {
		case -1:
			response.append("421 ");
			response.append(cl.nick);
			response.append(" ");
			response.append(msg.cmd);
			response.append(" :Unknown command\r\n");
			break;
		case 0: // PASS
			if (msg.cmd.length() && cl.auth == false && cl.registered == false)
				cl.auth = true;
			cout << "PASS: " << msg.params << endl;
			break;
		case 1: // NICK
			if (msg.params.length()) {
				cl.nick = msg.params;
			}
			cout << "NICK: " << cl.nick << endl;
			break;
		case 2: // USER must check num of params correct
			if (msg.params.length() && msg.trailing.length() && cl.nick.length()) {
				cl.user = msg.params.substr(0, msg.params.find_first_of(' '));
				cl.host = msg.params.substr(msg.params.find_first_of(' ') + 1 , msg.params.find_first_of(' ', msg.params.find_first_of(' ') + 1) - msg.params.find_first_of(' '));
				cout << "\tCL NICK:" << cl.nick << endl;
				cl.fullname.append(cl.nick);
				cl.fullname.append("!");
				cl.fullname.append(cl.user);
				cl.fullname.append("@");
				cl.fullname.append("localhost");
				cout << endl << "FULLNAME: " << cl.fullname << endl;
			}
			cout << "\tUSER: " << cl.user << endl;
			cout << "\tNICK: " << cl.nick << endl;
			cout << "\tHOST: " << cl.host << endl;
			cout << "\tMSG: " << msg.cmd << " PARAMS: " << msg.params << endl;
			cout << "\tFULLNAME: " << cl.fullname << endl;
			if (cl.auth && cl.nick.length() && cl.host.length() && cl.user.length() && !cl.registered) {
				cl.registered = true;
				response = ":localhost 001 ";
				response.append(cl.nick);
				response.append(" :Welcome to the Internet Relay Network ");
				response.append(cl.fullname);
				response.append("\r\n");
			} else {
				cout << "auth: " << cl.auth << " nick: " << cl.nick << " host: " << cl.host << " user " << cl.user << endl;
			}
			break;
		default:
			break;
	}
	if (response.length()) {

		cout << "RESPONSE: " << response << endl;
		string hardcoded = ":localhost 001 tjegades :Welcome to the Internet Relay Network tjegades!tjegades@localhost\r\n";
		cout << "RESPONSE: " << hardcoded << endl;
		send(cl.socket, response.c_str(), response.length(), 0);
		response.erase();
	}
}

//:localhost 001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>

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

	std::map<int, Client> clients;

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

					// Get client IP address
					char ip_str[INET_ADDRSTRLEN];
					convertInAddrToString(remote.sin_addr, ip_str, sizeof(ip_str));

					cout << "client " << fd_count << " at fd " << fds[fd_count].fd << " and ip " << ip_str << " joined!" << endl;
					Client client(client_socket, ip_str);
					clients.insert(std::pair<int, Client>(client_socket, client));
					// send(clients.at(client_socket).socket, clients.at(client_socket).ip_addr.c_str(), sizeof ip_str, 0);
					// send(clients.at(client_socket).socket, " $> ", 4, 0);

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
						Client &cl = clients.at(fds[i].fd);
						// parse message into command
						cout << "received " << buffer << " from client " << i << " at fd " << fds[i].fd << endl;
						string message(buffer);
						if (cl.remainder.length()) {
							message.insert(0, cl.remainder);
						}
						if (!message.empty()) {
							size_t idx = message.find("\r\n", 0);
							while (idx != string::npos) {
								cout << "idx: " << idx << endl;
								string cmd = message.substr(0, idx);
								cout << "cmd: " << cmd << endl;

								/**
								 * TODO: execute command if valid
								 * before proceeding to next command
								*/
						
								execute_cmd(cl, cmd);


								message.erase(0, idx + 2);
								// cout << "message: " << message << endl;

								idx = message.find("\r\n", 0);
							}
							if (!message.empty()) {
								cl.remainder = message;
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

	cout << server_socket << endl;
	cout << fd_count << endl;
	cout << "EXITING" << endl;
	
	for (size_t i = 0; i < fd_count; ++i)
		close(fds[i].fd);
}
