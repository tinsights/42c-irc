/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdutils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:43:21 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/20 14:34:23 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"


#define NUM_CMDS 8
#define PASSWD "hitchhiker"

// RPL_001 welcome msg template
// :localhost 001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>CRLF

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
					if (!cl.auth) {
						response.append("464 * :Password incorrect");
					}
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
				/**
				 * TODO:
				 * - !!!! validate params as valid nick
				 * - for e.g. cant be channel name, else later control flow will be confused
				 * - check protocol for more info
				 * - rfc2812 states: nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
				 * 
				*/
			if (!cl.auth) {
				/**
				 * Unsure if should send anything back (cybersec: can guess pw)
				 * but want to differentiate between clients / server hanging
				 * and other incorrect behaviour
				 * 
				 * i.e. every message should have some sort of response. good for debugging
				*/
				response.append("451 ");
				response.append(" * :You have not registered"); // * is nick
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
			} else {
				response.append("431 :No nickname given");
				break;
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
				/**
				 * TODO: 
				 * - upstream format check?
				 * - valid names
				 * format: <username> <hostname> <servername> :<realname>
				 * 
				*/
			if (!cl.auth) {
				/**
				 * Unsure if should send anything back (cybersec: can guess pw)
				 * but want to differentiate between clients / server hanging
				 * and other incorrect behaviour
				 * 
				 * i.e. every message should have some sort of response. good for debugging
				*/
				response.append("451 ");
				response.append(" * :You have not registered"); // * is nick
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
					} else if (Channel::channel_list.find(recpt) != Channel::channel_list.end()) { 
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
						std::set<string>::iterator it = Channel::channel_list.at(recpt).users.begin();
						while (it != Channel::channel_list.at(recpt).users.end()) {
							if (*it != cl.nick) {
								try {
									int socket = Client::client_list.at(*it).socket;
									send(socket, message.c_str(), message.length(), 0);
								} catch (std::exception const & e) {
									YEET BOLDYELLOW << "OOPSIE: " << *it << " ";
									YEET BOLDYELLOW << e.what() ENDL;
								}

							}
							++it;
							YEET "looping" ENDL;
						}
					} else {
						response.append("401 ");
						response.append(cl.nick);
						response.append(" :No such nick (");
						response.append(recpt);
						response.append(")");
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
				/**
				 * Unsure if should send anything back (cybersec: can guess pw)
				 * but want to differentiate between clients / server hanging
				 * and other incorrect behaviour
				 * 
				 * i.e. every message should have some sort of response. good for debugging
				*/
				response.append("451 ");
				response.append(" * :You have not registered"); // * is nick
				break;
			}
			if (msg.params.length()) {
				if (Channel::channel_list.find(msg.params) != Channel::channel_list.end()) {
					// channel currently exists
					if (Channel::channel_list.at(msg.params).users.find(cl.nick) != Channel::channel_list.at(msg.params).users.end() ){
						// user already in channel
						break;
					}
					Channel::channel_list.at(msg.params).users.insert(cl.nick);
					cl.joined_channels.insert(msg.params);
				} else {
					// channel doesnt exist
					Channel *newchnl = new Channel(msg.params);
					Channel::channel_list.insert(std::pair<string, Channel & >(msg.params, *newchnl));
					newchnl->users.insert(cl.nick);
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

			/**
			 * MAY NEED TO DO FOR Netcat
			 * Even though subject does not require QUIT command
			 * good to do? maybe can differentiate between irssi clients
			 * and nc client based on CAP LS first command
			 * 
			*/
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
