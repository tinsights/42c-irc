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
#include <vector>
#include <set>
#include <algorithm>
#include <sstream>
#include <iomanip>
using std::setw;


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
		Client(int fd, string ip) : socket(fd), ip_addr(ip), nick(""), user(""), host(""), server(""), realname(""), fullname(""), remainder(""), auth(false), registered(false) {};
		// TODO: copy
		~Client() {};
		int 	socket;
		string	ip_addr;
		string	nick;
		string	user;
		string	host;
		string	server;
		string	realname;
		string	fullname;
		string	remainder;
		bool	auth;
		bool	registered;

		std::set<string> joined_channels;

		static std::map<string, Client &> client_list;
		static std::map<string, std::set<string> > channels;

	private:
		Client() {};
		
};
// should be in .cpp
std::map<string, Client &> Client::client_list;
std::map<string, std::set<string> > Client::channels;

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
		YEET BOLDRED << setw(20) << "Prefix: " << prefix ENDL;
		YEET BOLDRED << setw(20) << "Cmd: " << cmd ENDL;
		YEET BOLDRED << setw(20) << "Params: " << params ENDL;
		YEET BOLDRED << setw(20) << "Trail: " << trailing ENDL;
		YEET "" ENDL;
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

#define NUM_CMDS 8
#define PASSWD "hitchhiker"

std::map<int, Client> connections;


void execute_cmd(Client &cl, string &cmd) {
	Message msg(cmd);
	string cmds[NUM_CMDS] = {"PASS", "NICK", "USER", "PRIVMSG", "JOIN", "QUIT", "MODE", "PING"};
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
	string servername = ":ft_irc ";
	string	response = servername;

	switch (index) {
		case -1:
			response.append("421 ");
			if (cl.nick.length())
				response.append(cl.nick);
			else
				response.append("*"); // can set to default
			response.append(" ");
			response.append(msg.cmd); 	
			response.append(" :Unknown command");
			break;
		case 0: // PASS
			YEET BOLDRED << "\tPASS: " << msg.params ENDL;

			if (msg.params.length()) {
				if (cl.registered == false) {
					cl.auth = msg.params == PASSWD;
				} else {
					// already registered
					response.append("462 ");
					response.append(cl.nick);
					response.append(" :You may not reregister");
				}
			} else {
				// not enough params
				response.append("461 ");
				response.append(cl.nick);
				response.append(" PASS :Not enough parameters");
			}
			break;
		case 1: // NICK
			if (!cl.auth) {
				break;
			}
			if (msg.params.length()) {
				if (Client::client_list.find(msg.params) != Client::client_list.end() ) {
					response.append("433 * ");
					response.append(msg.params);
					response.append(" :Nickname is already in use.");
				} else {
					cl.nick = msg.params;
				}
			}
			// cout << "\tNICK: " << cl.nick << endl;
			if (cl.auth && cl.nick.length() && cl.host.length() && cl.user.length() && !cl.registered) {
				string fullname = "";
				fullname.append(cl.nick);
				fullname.append("!");
				fullname.append(cl.user);
				fullname.append("@");
				fullname.append(cl.ip_addr);
				cl.fullname = fullname;
				cl.registered = true;
				Client::client_list.insert(std::pair<string, Client &>(cl.nick, cl));
				response.append("001 ");
				response.append(cl.nick);
				response.append(" :Welcome to the Internet Relay Network ");
				response.append(cl.fullname);
			} else {
				// cout << "auth: " << cl.auth << " nick: " << cl.nick << " host: " << cl.host << " user " << cl.user << endl;
			}
			break;
		case 2: // USER 
				// must check num of params correct
				// ONLY SINGLE SPACES
				// <username> <hostname> <servername> :<realname>
			if (!cl.auth) {
				break;
			}
			if (msg.params.length() && msg.trailing.length()) {
				// size_t space_idx = msg.params.find_first_of(' ');
				// cl.user = msg.params.substr(0, space_idx);
				// cl.host = msg.params.substr(space_idx + 1 , msg.params.find_first_of(' ', space_idx) - space_idx);
				std::stringstream ss(msg.params);
				ss >> cl.user;
				ss >> cl.host;
				ss >> cl.server;
				cl.realname = msg.trailing;
			}
			YEET setw(20) << "USER: " << cl.user ENDL;
			YEET setw(20) << "HOST: " << cl.host ENDL;
			YEET setw(20) << "SERVER: " << cl.server ENDL; // confusing
			YEET setw(20) << "REALNAME: " << cl.realname ENDL;

			if (cl.auth && cl.nick.length() && cl.host.length() && cl.user.length() && !cl.registered) {
				string fullname = "";
				fullname.append(cl.nick);
				fullname.append("!");
				fullname.append(cl.user);
				fullname.append("@");
				fullname.append(cl.ip_addr);
				cl.fullname = fullname;
				cl.registered = true;
				Client::client_list.insert(std::pair<string, Client &>(cl.nick, cl));
				response.append("001 ");
				response.append(cl.nick);
				response.append(" :Welcome to the Internet Relay Network ");
				response.append(cl.fullname);
			} else {
				// cout << "auth: " << cl.auth << " nick: " << cl.nick << " host: " << cl.host << " user " << cl.user << endl;
			}
			break;
		case 3: // PRIVMSG
			if (msg.params.length()) {
				if (msg.params.find_first_of(' ') != string::npos) {
					string recpt = msg.params.substr(0, msg.params.find_first_of(' '));
					if (Client::client_list.find(recpt) != Client::client_list.end()) {
						string message;
						message
							.append(":")
							.append(cl.fullname)
							.append(" PRIVMSG ")
							.append(recpt)
							.append(" :")
							.append(msg.trailing)
							.append("\r\n");
						send(Client::client_list.at(recpt).socket, message.c_str(), message.length(), 0);
						break;
					} else if (Client::channels.find(recpt) != Client::channels.end()) { 
						// send to channel
						string message;
						message
							.append(":")
							.append(cl.fullname)
							.append(" PRIVMSG ")
							.append(recpt)
							.append(" :")
							.append(msg.trailing)
							.append("\r\n");
						std::set<string>::iterator it = Client::channels.at(recpt).begin();
						while (it != Client::channels.at(recpt).end()) {
							if (*it != cl.nick) {
								try {
									int socket = Client::client_list.at(*it).socket;
									send(socket, message.c_str(), message.length(), 0);
								} catch (std::exception const & e) {
									YEET BOLDYELLOW << "OOPSIE: ";
									YEET BOLDYELLOW << e.what() ENDL;
								}

							}
							++it;
							YEET "looping" ENDL;
						}
					} else {
						response.append("401 ");
						response.append(cl.nick);
						// response.append(" :No such nick (");
						response.append(recpt);
						// response.append(")");
					}
				}
				else {
					response.append("412 ");
					response.append(cl.nick);
					response.append(" :No text to send");
				}

			} else {
				response.append("411 ");
				response.append(cl.nick);
				response.append(" :No recipient given (PRIVMSG)");
			}
			break;
		case 4: // JOIN
		/**
		 * TODO: validate chnl name i.e. params
		*/
			if (!cl.auth) {
				response.append("451 ");
				response.append(" * :You have not registered"); // * is nick
				break;
			}
			if (msg.params.length()) {
				if (Client::channels.find(msg.params) != Client::channels.end()) {
					// channel currently exists
					if (Client::channels.at(msg.params).find(cl.nick) != Client::channels.at(msg.params).end() ){
						// user already in channel
						break;
					}
					Client::channels[msg.params].insert(cl.nick);
					cl.joined_channels.insert(msg.params);
				} else {
					// channel doesnt exist
					std::set<string> newchn;
					newchn.insert(cl.nick);
					Client::channels.insert(std::pair<string, std::set<string> >(msg.params, newchn));
					cl.joined_channels.insert(msg.params);
				}
			} else {
				// not enough params
				response.append("461 ");
				response.append(cl.nick);
				response.append(" JOIN :Not enough parameters");
				break;
			}
			cout << cl.nick << " has joined " << msg.params << endl;
			break;
		case 5: // QUIT
				// :tj!~tj@203.149.201.178 QUIT :Client Quit
			response.append(servername);
			response.append(" :Closing Link: (Client Quit) ");
			send(cl.socket, response.c_str(), response.length(), 0);
			response.erase(response.begin(), response.end());

			// Client::client_list.erase(cl.nick);
			// close(cl.socket);
			// connections.erase(cl.socket);
			break;
		default:
			break;
	}
	if (response.length() > servername.length()) {
		response.append("\r\n");
		YEET BOLDGREEN << setw(20) << "RESPONSE: " << response ENDL;
		send(cl.socket, response.c_str(), response.length(), 0);
		response.erase();
	}
}

//:localhost 001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>
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

	while (server_running) {
		int poll_count = poll(fds, fd_count, -1);
		
		for (size_t i = 0; i < fd_count; ++i) {
			if (fds[i].revents & POLLIN) {
				YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
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
					
					for (size_t j = 1; j <= fd_count; ++j) {
						if (fds[j].fd == 0) {
							fds[j].fd = client_socket; // <--- culprit
							fds[j].events = POLLIN;
						}
					}

					// Get client IP address
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
						*/
						Client cl = connections.at(fds[i].fd);
						

						if (cl.auth && cl.nick.length()) {
							// remove from all channels
							for (std::set<string>::iterator it = cl.joined_channels.begin(); it != cl.joined_channels.end(); ++it) {
								
								try {
									Client::channels[*it].erase(cl.nick);
									YEET BOLDBLUE << "Removed " << cl.nick << " from " << *it ENDL;
								}
								catch (std::exception const & e) {
									YEET BOLDRED << "Unable to remove " << cl.nick << " from " << *it ENDL;
								}
							}
							// remove from nickname list
							Client::client_list.erase(cl.nick);
						}

						// try {
						// 	Client::client_list.erase(connections.at(fds[i].fd).nick);
						// }
						// catch (std::exception const & e) {
						// 	std::cerr << e.what() << endl; // most likely no nick.
						// }
				
						close(fds[i].fd);
						connections.erase(fds[i].fd);
						fds[i].fd = 0;
						fd_count--;
						YEET BOLDRED << "FD COUNT: " << fd_count ENDL;
					} else {
						Client &cl = connections.at(fds[i].fd);
						// parse message into command
						YEET "\nRECVD FROM CLIENT " << i << " at fd " << fds[i].fd  << "\n" << BOLDBLUE << buffer ENDL;
						string message(buffer);
						if (cl.remainder.length()) {
							message.insert(0, cl.remainder);
						}
						if (!message.empty()) {
							size_t idx = message.find("\r\n", 0);
							while (idx != string::npos) {
								// YEET "idx: " << idx ENDL;
								string cmd = message.substr(0, idx);
								// YEET "cmd: " << cmd ENDL;

								/**
								 * TODO: execute command if valid
								 * before proceeding to next command
								 * :DONE:
								*/
						
								execute_cmd(cl, cmd);
								// YEET "after exec cmd";

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
