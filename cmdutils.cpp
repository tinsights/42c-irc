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

#define NUM_CMDS 11
#define PASSWD "hitchhiker"

// RPL_001 welcome msg template
// :localhost 001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>CRLF

void execute_cmd(Client &cl, Message & msg) {
	// Message msg(cmd);
	string cmds[NUM_CMDS] = {"PASS", "NICK", "USER", "PRIVMSG", "JOIN", "QUIT", "KICK", "TOPIC", "INVITE", "MODE", "PING"};
	int index = -1;
	for (int i = 0; i < NUM_CMDS; ++i) {
		if (msg.cmd == cmds[i]) {
			index = i;
			break;
		}
	}

	// upstream msg validity check??
	string servername = ":ft_irc "; // thats us baby. maybe can use gethostname or get our ip addr
	string	response = servername;

	switch (index) {
		case -1:
			// bad cmd
			response.append("421 ");
			if (cl.nick.length())
				response.append(cl.nick);
			else
				response.append("*"); // can set to default
			response.append(" ");
			response.append(msg.cmd); 	
			response.append(" :Unknown command");
			break;
		case 0:
			/** PASS
			 * Potential replies:
			 * 		ERR_NEEDMOREPARAMS 		461 DONE
			 * 		ERR_ALREADYREGISTERED 	462 DONE
			 * 		ERR_PASSWDMISMATCH		464 DONE
			 * 
			*/
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
				response.append(" * :You have not registered"); // * is nick placeholder
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
			/* Client can send either NICK followed by USER, so either can trigger below, which should be refactored away: */
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
				 * - write more comments for xf
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
				response.append(" * :You have not registered"); // * is nick placeholder
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
			} else {
				// ERR_NEEDMOREPARAMS	461
				response.append("461 ")
					.append(cl.nick)
					.append(" USER :Not enough parameters");
			}
			YEET setw(20) << "USER: " << cl.user ENDL;
			YEET setw(20) << "HOST: " << cl.host ENDL;
			YEET setw(20) << "SERVER: " << cl.server ENDL; // confusing. server is us
			YEET setw(20) << "REALNAME: " << cl.realname ENDL;

			/* Client can send either NICK followed by USER, so either can trigger below, which should be refactored away: */
			if (cl.auth && cl.nick.length() && cl.host.length() && cl.user.length() && !cl.registered) {
				string fullname = "";
				fullname.append(cl.nick)
					.append("!")
					.append(cl.user)
					.append("@")
					.append(cl.ip_addr);

				cl.fullname = fullname;
				cl.registered = true;

				Client::client_list.insert(std::pair<string, Client &>(cl.nick, cl));
				response.append("001 ")
					.append(cl.nick)
					.append(" :Welcome to the Internet Relay Network ")
					.append(cl.fullname);
			} else {
				// cout << "auth: " << cl.auth << " nick: " << cl.nick << " host: " << cl.host << " user " << cl.user << endl;
			}
			break;
		case 3: // PRIVMSG
				/**
				 * TODO: 
				 * - write more comments for xf
				 * 
				*/
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
			 *  Potential replies:
			 * 		ERR_NEEDMOREPARAMS	461
			 * 		ERR_BADCHANMASK 	476 <-- channel format wrong
			 * 		ERR_BADCHANNELKEY 	475 <-- wrong channel password, if req
			 * 		ERR_INVITEONLYCHAN	473
			 * 
			 * 		RPL_TOPIC 332 && RPL_NAMEREPLY 353 <-- on success
			 * 		DONE:	:server 353 NICK = CHNLNAME :list of nicknames // honestly what is the equals sign? idk im following liberachat
			 *	 			:server 366 NICK CHNLNAME :End of /NAMES list.T
			 				TODO: add @ prefix for ops
			 * 
			 *  TODO:
			 * 		- validate chnl name i.e. params BADCHANMASK
			 * 		- channel restrictions i.e. BADCHANNELKEY / INVITEONLYCHAN
			 * 	DOING:
			 * 		 - send back RPL_TOPIC and RPL_NAMEREPLY
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
				response.append(" * :You have not registered"); // * is nick placeholder
				break;
			}
			if (msg.params.length()) {
				if (Channel::channel_list.find(msg.params) != Channel::channel_list.end()) {
					// channel currently exists
					Channel & chnl = Channel::channel_list.at(msg.params);

					if (chnl.users.find(cl.nick) != chnl.users.end() ){
						// user already in channel
						// should send err msg
						// apparently IRC servers do nothing here
						break;
					}
					chnl.users.insert(cl.nick);
					cl.joined_channels.insert(msg.params);
					// for other users 
					string announcement = ":";
					announcement.append(cl.fullname)
								.append(" JOIN ")
								.append(msg.params)
								.append("\r\n");
					// create response as per :server 353 NICK = CHNNLNAME :LIST OF USERS
					response.append("353 ")
							.append(cl.nick)
							.append(" = ")
							.append(msg.params)
							.append(" :");
					std::set<string>::iterator it = chnl.users.begin();
					while (it != chnl.users.end()) {
						// send annoucnement message to all users in channel
						// as per :nick!~realname@servername JOIN #1
						send(Client::client_list.at(*it).socket, announcement.c_str(), announcement.length(), 0); // prob should be a method of a class somewhere
						// add @ prefix for ops
						if (chnl.opers.find(*it) != chnl.opers.end()) {
							response.append("@");
						}
						response.append(*it)
								.append(" ");
						++it;
					}
					// add CRLF and end of name list as per
					// :server 366 nick CHNLNAME :End of /NAMES list.
					response.append("\r\n")
							.append(servername)
							.append("366 ")
							.append(cl.nick)
							.append(" ")
							.append(msg.params)
							.append(" :End of /NAMES list.");
					// add RPL_TOPIC
					if (chnl.topic.length()) {
						response.append("\r\n")
								.append("332 ")
								.append(cl.nick)
								.append(" ")
								.append(msg.params)
								.append(" :")
								.append(chnl.topic);
					}
				} else {
					// channel doesnt exist
					Channel *newchnl = new Channel(msg.params); // heap memory must be cleared. unless we set a static channel limit in main.
					Channel::channel_list.insert(std::pair<string, Channel & >(msg.params, *newchnl));
					// add user to opers list
					newchnl->users.insert(cl.nick);
					newchnl->opers.insert(cl.nick); // should be a public setter that access private members and checks that user is in channel.
					cl.joined_channels.insert(msg.params);
					// create response as per :server 353 NICK = CHNNLNAME :LIST OF USERS
					response.append("353 ")
							.append(cl.nick)
							.append(" = ")
							.append(msg.params)
							.append(" :");
					// we know the only user in the channel is the one who joined
					// and they are an oper
					response.append("@")
							.append(cl.nick)
							.append(" ");
					// add CRLF and end of name list as per
					// :server 366 nick CHNLNAME :End of /NAMES list.
					response.append("\r\n")
							.append(servername)
							.append("366 ")
							.append(cl.nick)
							.append(" ")
							.append(msg.params)
							.append(" :End of /NAMES list.");
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
			/** 
			 * Response:
			 * 	:<nick>!<user>>@<host> QUIT :Client Quit
			*/
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
		case 6: // KICK (in progress)	
			/**
			 *  Potential replies:
			 * 		ERR_NEEDMOREPARAMS 461
			 * 		ERR_NOSUCHCHANNEL 403
			 * 		ERR_BADCHANMASK  476 <--> bad channel format
			 * 		ERR_NOTONCHANNEL 442
			 * 		ERR_CHANOPRIVSNEEDED 482 <-- not operator
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
				response.append(" * :You have not registered"); // * is nick placeholder
				break;
			}
			if (msg.params.length()) { // <channel> <user>
				/** TODO: 
				 * 	- check if both channel and user present in params (netcat proofing)
				 * 	- check if channel legit
				 *  - check if user is in channel and oper
				 *  - kick! 
				*/

			} else {
				// not enough params
				response.append("461 ");
				response.append(cl.nick);
				response.append(" KICK :Not enough parameters");
				break;
			}
		case 7: // TOPIC
			/**
			 * TODO: Implement TOPIC command functionality
			 * - Check if user is in a channel
			 * - Set the topic for the channel
			 * - Respond with appropriate messages
			 */
			// set topic if user is oper
			// send topic to all users in channel
			// send RPL_TOPIC to user
			if (!cl.auth) {
				/**
				 * Unsure if should send anything back (cybersec: can guess pw)
				 * but want to differentiate between clients / server hanging
				 * and other incorrect behaviour
				 * 
				 * i.e. every message should have some sort of response. good for debugging
				*/
				response.append("451 ");
				response.append(" * :You have not registered"); // * is nick placeholder
				break;
			}

			if (msg.params.length()) {
				// find channel if exists
				if (Channel::channel_list.find(msg.params) == Channel::channel_list.end()) {
					// ERR_NOSUCHCHANNEL 403
					response.append("403 ");
					response.append(cl.nick);
					response.append(" :No such channel");
					break;
				}
				Channel & chnl = Channel::channel_list.at(msg.params);

				if (msg.trailing.length() == 0) { // getting topic
					if (chnl.topic.length()) {
						// send RPL_TOPIC to user
						response.append("332 ")
								.append(cl.nick)
								.append(" ")
								.append(msg.params)
								.append(" :")
								.append(chnl.topic);
					} else {
						// ERR_NOTOPIC 331
						response.append("331 ");
						response.append(cl.nick);
						response.append(" ");
						response.append(msg.params);
						response.append(" :No topic is set");
					}
					break;
				}
				else if (!chnl.topic_protected || chnl.opers.find(cl.nick) != chnl.opers.end()) { // setting topic if oper
						chnl.topic = msg.trailing;
						// send topic to all users in channel
						string announcement = ":";
						announcement.append(cl.fullname)
									.append(" TOPIC ")
									.append(msg.params)
									.append(" :")
									.append(msg.trailing)
									.append("\r\n");
						std::set<string>::iterator it = chnl.users.begin();
						while (it != chnl.users.end()) {
							send(Client::client_list.at(*it).socket, announcement.c_str(), announcement.length(), 0);
							++it;
						}
						// send RPL_TOPIC to user
						response.append("332 ")
								.append(cl.nick)
								.append(" ")
								.append(msg.params)
								.append(" :")
								.append(chnl.topic);
					} else {
						// ERR_CHANOPRIVSNEEDED 482
						response.append("482 ");
						response.append(cl.nick);
						response.append(" :You're not a channel operator");
					}
				} 
			else {
				// not enough params
				response.append("461 ");
				response.append(cl.nick);
				response.append(" TOPIC :Not enough parameters");
				break;
			}
			break;
		case 8: // INVITE
			/**
			 * TODO: Implement INVITE command functionality
			 * - Check if user is in a channel
			 * - Check if the invited user exists and is not already in the channel
			 * - Send an invitation to the user
			 * - Respond with appropriate messages
			 */
			break;
		case 9: //MODE
			/**
			 * TODO: Implement MODE command functionality
			 * - Check if user is in a channel
			 * - Check if the user is an operator
			 * - Set the mode for the channel
			 * - Respond with appropriate messages
			 */
			// get chnl and mode from params
			break;
		default:
			// Existing cases...
			// currently ignore PING and MODE that are automatically sent by irssi
			// so as to avoid confusing "PING: unknown command"	 on client side
			break;
	}
	if (response.length() > servername.length()) {
		response.append("\r\n"); // if we have a response class, response.send(Client & cl)) can append CRLF
		YEET BOLDGREEN << setw(20) << "RESPONSE: " << response ENDL;
		send(cl.socket, response.c_str(), response.length(), 0);
		response.erase();
	}
}
