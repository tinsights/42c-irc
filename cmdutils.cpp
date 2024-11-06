/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cmdutils.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:43:21 by tjegades          #+#    #+#             */
/*   Updated: 2024/11/06 03:23:31 by xlow             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "irc.hpp"

#include "Client.hpp"
#include "Message.hpp"
#include "Channel.hpp"

#define NUM_CMDS 11

void execute_cmd(Client &cl, Message & msg) {
	string cmds[NUM_CMDS] = {"PASS", "NICK", "USER", "PRIVMSG", "JOIN", "QUIT", "KICK", "TOPIC", "INVITE", "MODE", "PING"};
	int index = -1;
	for (int i = 0; i < NUM_CMDS; ++i) {
		if (msg.cmd == cmds[i]) {
			index = i;
			break;
		}
	}
	string 	servername = ":ft_irc ";
	string	response = servername;

	switch (index) {
		case -1:
			response.append("421 ");
			if (cl.nick.length()) 	response.append(cl.nick);
			else 					response.append("*");
			response.append(msg.cmd + " :Unknown command");
			break;
		case 0: // PASS
			YEET BOLDRED << "\tPASS: " << msg.params ENDL;
			if (msg.params.length()) {
				if (cl.registered == false) {
					cl.auth = msg.params == Client::password;
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
			if (!cl.auth) {
				response.append("451 " + cl.nick + " * :You have not registered");
				response.append(" * :You have not registered");
				break;
			}
			if (msg.params.length()) {
				if (!Client::is_valid_nick(msg.params)) {
					response.append("432 * ");
					response.append(msg.params);
					response.append(" :Erroneous nickname");
					break;
				}
				else if (Client::client_list.find(msg.params) != Client::client_list.end() ) {
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
			
			if (cl.ready_to_register()) {
				cl.fullname = cl.nick + "!" + cl.user + "@" + cl.ip_addr;
				cl.registered = true;
				Client::client_list.insert(std::pair<string, Client &>(cl.nick, cl));
				response.append("001 " + cl.nick + " :Welcome to the Internet Relay Network " + cl.fullname);
			}
			break;
		case 2: // USER
			if (!cl.auth) {
				response.append("451 " + cl.nick + " * :You have not registered");
				response.append(" * :You have not registered");
				break;
			}
			if (msg.params.length() && msg.trailing.length()) {
				std::stringstream ss(msg.params);
				ss >> cl.user;
				if (Client::is_valid_user(cl.user) == false) {
					response.append("501 ");
					response.append(cl.nick);
					response.append(" :Invalid username");
					cl.user.clear();
					break;
				}
				ss >> cl.host;
				ss >> cl.server;
				cl.realname = msg.trailing;
			} else {
				response.append("461 " + cl.nick + " USER :Not enough parameters");
			}
			
			if (cl.ready_to_register()) {
				cl.fullname = (cl.nick + "!" + cl.user + "@" + cl.ip_addr);
				cl.registered = true;
				Client::client_list.insert(std::pair<string, Client &>(cl.nick, cl));
				response.append("001 " + cl.nick + " :Welcome to the Internet Relay Network " + cl.fullname);
			}
			break;
		case 3: // PRIVMSG
			if (msg.params.length()) {
				string recpt = msg.param_list[0];
				string privmsg = msg.trailing;
				if (privmsg.length()) {
					if (Client::client_list.find(recpt) != Client::client_list.end()) {
						string message = ":" + cl.fullname + " PRIVMSG " + recpt + " :" + msg.trailing + "\r\n";
						send(Client::client_list.at(recpt).socket, message.c_str(), message.length(), 0);
						break;
					} else if (Channel::channel_list.find(recpt) != Channel::channel_list.end()){
						// check that user is in channel
						Channel & chnl = Channel::channel_list.at(recpt);
						if (chnl.users.find(cl.nick) == chnl.users.end()) {
							response.append("404 " + cl.nick + " " + recpt + " :Cannot send to channel");
							break;
						}
						string message = ":" + cl.fullname + " PRIVMSG " + recpt + " :" + msg.trailing + "\r\n";
						std::set<string>::iterator it = Channel::channel_list.at(recpt).users.begin();
						while (it != Channel::channel_list.at(recpt).users.end()) {
							if (*it != cl.nick) {
								try {
									int socket = Client::client_list.at(*it).socket;
									send(socket, message.c_str(), message.length(), 0);
								} catch (std::exception const & e) {
									YEET BOLDYELLOW << e.what() ENDL;
								}
							}
							++it;
						}
					} else {
						response.append("401 " + cl.nick + " " + recpt + " :No such nick/channel");
					}
				}
				else {
					response.append("412 " + cl.nick + " :No text to send");
				}

			} else {
				response.append("411 " + cl.nick + " PRIVMSG :No recipient given");
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
			if (!cl.registered) {
				response.append("451 " + cl.nick + " * :You have not registered");
				response.append(" * :You have not registered");
				break;
			}
			if (msg.params.length()) {
				if (Channel::is_valid_channel(msg.params) == false) {
					response.append("476 " + cl.nick + " :Bad channel mask");
					break;
				}
				else if (Channel::channel_list.find(msg.params) != Channel::channel_list.end()) {
					Channel & chnl = Channel::channel_list.at(msg.params);

					if (chnl.users.find(cl.nick) != chnl.users.end() ){
						break;
					}
					else if (chnl.invite_only && chnl.invited.find(cl.nick) == chnl.invited.end()) {
						// user not invited
						response.append("473 " + cl.nick + " " + msg.params + " :Cannot join channel, requires invite. (+i)");
						break;
					}
					else if (chnl.passwd_protected) {
						if (msg.param_list.size() < 2) {
							// channel has password
							// send ERR_BADCHANNELKEY
							response.append("475 " + cl.nick + " " + msg.params + " :Cannot join channel, requires password. (+k)");
							break;
						} else if (msg.param_list[1] != chnl.passwd) {
							// wrong password
							response.append("464 " + cl.nick + " " + msg.params + " :Password incorrect.");
							break;
						}
						break;
					}
					else if (chnl.user_limit && chnl.users.size() >= chnl.user_limit) {
						// channel is full
						response.append("471 " + cl.nick + " " + msg.params + " :Cannot join channel, channel is full.");
						break;
					}
					chnl.users.insert(cl.nick);
					cl.joined_channels.insert(msg.params);
					// for other users
					string announcement = ":";
					announcement.append(cl.fullname + " JOIN " + msg.params + "\r\n");
					// create response as per :server 353 NICK = CHNNLNAME :LIST OF USERS
					response.append("353 " + cl.nick + " = " + msg.params + " :");
					std::set<string>::iterator it = chnl.users.begin();
					while (it != chnl.users.end()) {
						// send annoucnement message to all users in channel
						// as per :nick!~realname@servername JOIN #1
						send(Client::client_list.at(*it).socket, announcement.c_str(), announcement.length(), 0); // prob should be a method of a class somewhere
						// add @ prefix for ops
						if (chnl.opers.find(*it) != chnl.opers.end()) {
							response.append("@");
						}
						response.append(*it + " ");
						++it;
					}
					// add CRLF and end of name list as per
					// :server 366 nick CHNLNAME :End of /NAMES list.
					response.append("\r\n" + servername + "366 " + cl.nick + " " + msg.params + " :End of /NAMES list.");
					// add RPL_TOPIC
					if (chnl.topic.length()) {
						response.append("\r\n" + servername + "332 " + cl.nick + " " + msg.params + " :" + chnl.topic);
					}
				} else {
					// channel doesnt exist
					Channel *newchnl = new Channel(msg.params);
					Channel::channel_list.insert(std::pair<string, Channel & >(msg.params, *newchnl));
					// add user to opers list
					newchnl->users.insert(cl.nick);
					newchnl->opers.insert(cl.nick); // should be a public setter that access private members and checks that user is in channel.
					cl.joined_channels.insert(msg.params);
					// create response as per :server 353 NICK = CHNNLNAME :LIST OF USERS
					response.append("353 " + cl.nick + " = " + msg.params + " :");
					// we know the only user in the channel is the one who joined
					// and they are an oper
					response.append("@" + cl.nick + " ");
					// add CRLF and end of name list as per
					// :server 366 nick CHNLNAME :End of /NAMES list.
					response.append("\r\n" + servername + "366 " + cl.nick + " " + msg.params + " :End of /NAMES list.");
				}
			} else {
				// not enough params
				response.append("461 " + cl.nick + " JOIN :Not enough parameters");
				break;
			}
			cout << cl.nick << " has joined " << msg.params << endl;
			break;
		case 5: // QUIT
			/**
			 * Response:
			 * 	:<nick>!<user>>@<host> QUIT :Client Quit
			*/
			response.append(servername + "QUIT :Client Quit");
			send(cl.socket, response.c_str(), response.length(), 0);
			response.erase(response.begin(), response.end());
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
			if (!cl.registered) {
				response.append("451 " + cl.nick + " * :You have not registered");
				break;
			}
			else if (msg.param_list.size() < 2) { // <channel> <user>
				// not enough params
				response.append("461 " + cl.nick + " KICK :Not enough parameters");
				break;
			}
			else {// get chnl and user from params
				string chnlname = msg.param_list[0];
				string user = msg.param_list[1];
				// check if channel exists
				if (Channel::channel_list.find(chnlname) == Channel::channel_list.end()) {
					response.append("403 " + cl.nick + " :No such channel");
					break;
				}
				Channel & chnl = Channel::channel_list.at(chnlname);
				// check if client in channel
				if (chnl.users.find(cl.nick) == chnl.users.end()) {
					response.append("442 " + cl.nick + " " + chnlname + " :You're not on that channel");
					break;
				}
				// check if this client is a chnl operator
				if (chnl.opers.find(cl.nick) == chnl.opers.end()) {
					response.append("482 " + cl.nick + " " + chnlname + " :You're not a channel operator");
					break;
				}				
				// send kick message to user
				string kickmsg = ":" + cl.fullname + " KICK " + chnlname + " " + user + msg.trailing + "\r\n";
				// send to all users in channel
				std::set<string>::iterator it = chnl.users.begin();
				while (it != chnl.users.end()) {
					send(Client::client_list.at(*it).socket, kickmsg.c_str(), kickmsg.length(), 0);
					++it;
				}
				// remove channel from user's joined channels
				cl.joined_channels.erase(chnlname);
				// kick user
				chnl.users.erase(user);
				// if user is in invite list, remove
				chnl.invited.erase(user);
				// if user is oper, remove
				chnl.opers.erase(user);
			}
			break;
		case 7: // TOPIC
			if (!cl.registered) {
				response.append("451 " + cl.nick + " * :You have not registered");
				break;
			}
			if (msg.params.length()) {
				// find channel if exists
				if (Channel::channel_list.find(msg.params) == Channel::channel_list.end()) {
					// ERR_NOSUCHCHANNEL 403
					response.append("403 " + cl.nick + " :No such channel");
					break;
				}
				Channel & chnl = Channel::channel_list.at(msg.params);
				if (msg.trailing.length() == 0) { // getting topic
					if (chnl.topic.length()) {
						// send RPL_TOPIC to user
						response.append("332 " + cl.nick + " " + msg.params + " :" + chnl.topic);
					} else {
						// ERR_NOTOPIC 331
						response.append("331 " + cl.nick + " " + msg.params + " :No topic is set");
					}
					break;
				}
				else if (!chnl.topic_protected || chnl.opers.find(cl.nick) != chnl.opers.end()) { // setting topic if oper
						chnl.topic = msg.trailing;
						// send topic to all users in channel
						string announcement = ":" + cl.fullname + " TOPIC " + msg.params + " :" + msg.trailing + "\r\n";
						std::set<string>::iterator it = chnl.users.begin();
						while (it != chnl.users.end()) {
							send(Client::client_list.at(*it).socket, announcement.c_str(), announcement.length(), 0);
							++it;
						}
						// send RPL_TOPIC to user
						response.append("332 " + cl.nick + " " + msg.params + " :" + chnl.topic);
					} else {
						// ERR_CHANOPRIVSNEEDED 482
						response.append("482 " + cl.nick + " " + msg.params + " :You're not a channel operator");
					}
				}
			else {
				// not enough params
				response.append("461 " + cl.nick + " TOPIC :Not enough parameters");
				break;
			}
			break;
		case 8: // INVITE
			if (!cl.registered) {
				response.append("451 " + cl.nick + " * :You have not registered");
				break;
			}
			else if (msg.param_list.size() < 2) {
				response.append("461 " + cl.nick + " INVITE :Not enough parameters");
				break;
			} else {
				// get chnl and user from params
				string chnlname = msg.param_list[1], user = msg.param_list[0];
				if (Channel::is_valid_channel(chnlname) == false) {
					response.append("476 " + cl.nick + " :Bad channel mask");
					break;
				}
				else if (Client::is_valid_nick(user) == false) {
					response.append("432 " + cl.nick + " " + user + " :Erroneous nickname");
					break;
				}
				// check if channel exists
				if (Channel::channel_list.find(chnlname) == Channel::channel_list.end()) {
					response.append("403 " + cl.nick + " :No such channel");
					break;
				}
				// check if user is in server
				if (Client::client_list.find(user) == Client::client_list.end()) {
					response.append("401 " + cl.nick + " " + user + " :No such nick/channel");
					break;
				}
				Channel & chnl = Channel::channel_list.at(chnlname);
				// check if user is in channel
				if (chnl.users.find(user) != chnl.users.end()) {
					response.append("443 " + cl.nick + " " + user + " " + chnlname + " :is already on channel");
					break;
				}
				// send invite
				else if (chnl.invited.find(user) != chnl.invited.end()) {
					response.append("443 " + cl.nick + " " + user + " " + chnlname + " :is already invited");
					break;
				}
				chnl.invited.insert(user);
			}
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
			if (!cl.registered) {
				response.append("451 " + cl.nick + " * :You have not registered");
				response.append(" * :You have not registered");
				break;
			}
			else if (msg.param_list.size()  < 2) {
				response.append("461 ");
				response.append(cl.nick);
				response.append(" MODE :Not enough parameters");
				break;
			} else {
				// split params into channel and mode
				string chnlname = msg.param_list[0];
				// if received user mode command as per irssi auto, ignore
				if (Client::client_list.find(chnlname) != Client::client_list.end()) {
					break;
				}
				if (Channel::is_valid_channel(chnlname) == false) {
					response.append("476 " + cl.nick + " :Bad channel mask");
					break;
				}
				string modes = msg.param_list[1];
				if (modes.length() == 0) {
					response.append("461 " + cl.nick + " MODE :Not enough parameters");
					break;
				}
				// check if channel exists
				if (Channel::channel_list.find(chnlname) == Channel::channel_list.end()) {
					response.append("403 " + cl.nick + " :No such channel");
					break;
				}
				Channel & chnl = Channel::channel_list.at(chnlname);
				// check if client is oper
				if (chnl.opers.find(cl.nick) == chnl.opers.end()) {
					response.append("482 " + cl.nick + " " + chnlname + " :You're not a channel operator");
					break;
				}
				// validate mode params. only accept i t k o and l
				// mode can be +i-t or +it, etc
				if (modes.find_first_not_of("+-itkol") != string::npos	// has other rubbish
					|| (modes[0] != '+' && modes[0] != '-')				// first char must be + or -
					|| modes.find_first_of("itkol") == string::npos)		//	one of i t k o must be present
				{
					response.append("501 " + cl.nick + " :Unknown MODE flag");
					break;
				}
				string mode_changes = "";
				bool add = true;
				bool prefixed = false;
				for (size_t i = 0; i < modes.length(); ++i) {
					while (modes[i] == '+' || modes[i] == '-') {
						prefixed = false;
						add = modes[i] == '+';
						++i;
					}
					if (add) {
						if (modes[i] == 'i' && !chnl.invite_only) {
							chnl.invite_only = true;
							if (!prefixed) {
								mode_changes.append("+");
								prefixed = true;
							}
							mode_changes.append("i");
						} else if (modes[i] == 't' && !chnl.topic_protected) {
							chnl.topic_protected = true;
							if (!prefixed) {
								mode_changes.append("+");
								prefixed = true;
							}
							mode_changes.append("t");
						} else if (modes[i] == 'k' && !chnl.passwd_protected) {
							if (msg.param_list.size() > 2) {
								chnl.passwd = msg.param_list[2];
								chnl.passwd_protected = true;
								if (!prefixed) {
									mode_changes.append("+");
									prefixed = true;
								}
								mode_changes.append("k");
							} else {
								response.append("461 ")
										.append(cl.nick)
										.append(" MODE :Not enough parameters");
								break;
							}
						} else if (modes[i] == 'o') {
							// add oper
							// check if next param is present
							if (msg.param_list.size() > 2) {
								string oper = msg.param_list[2];
								if (chnl.opers.find(oper) != chnl.opers.end()) {
									response.append("441 " + cl.nick + " " + oper + " :They are already an operator");
									break;
								}
								else if (chnl.users.find(oper) != chnl.users.end()) {
									chnl.opers.insert(oper);
									if (!prefixed) {
										mode_changes.append("+");
										prefixed = true;
									}
									mode_changes.append("o " + oper);
								} else {
									response.append("441 " + cl.nick + " " + oper + " :They aren't on that channel");
									break;
								}
							} else {
								response.append("461 " + cl.nick + " MODE :Not enough parameters");
								break;
							} 
						} else if (modes[i] == 'l') {
							// add limit
							if (msg.param_list.size() > 2) {
								int limit = std::atoi(msg.param_list[2].c_str());
								if (limit <= 0)
									break;
								chnl.user_limit = limit;
								if (!prefixed) {
									mode_changes.append("+");
									prefixed = true;
								}
								std::stringstream ss;
								ss << limit;
								mode_changes.append("l " + ss.str());
							} else {
								response.append("461 " + cl.nick + " MODE :Not enough parameters");
								break;
							}
						}
					} else {
						if (modes[i] == 'i' && chnl.invite_only) {
							chnl.invite_only = false;
							if (!prefixed) {
								mode_changes.append("-");
								prefixed = true;
							}
							mode_changes.append("i");
						} else if (modes[i] == 't' && chnl.topic_protected) {
							chnl.topic_protected = false;
							if (!prefixed) {
								mode_changes.append("-");
								prefixed = true;
							}
							mode_changes.append("t");
						} else if (modes[i] == 'k' && chnl.passwd_protected) {
							chnl.passwd_protected = false;
							chnl.passwd.clear();
							if (!prefixed) {
								mode_changes.append("-");
								prefixed = true;
							}
							mode_changes.append("k");
						} else if (modes[i] == 'o') {
							// remove oper
							// check if next param is present
							if (msg.param_list.size() > 2) {
								string oper = msg.param_list[2];
								if (chnl.opers.find(oper) != chnl.opers.end()) {
									chnl.opers.erase(oper);
									if (!prefixed) {
										mode_changes.append("-");
										prefixed = true;
									}
									mode_changes.append("o " + oper);
								} else {
									response.append("441 " + cl.nick + " ");
									break;
								}
							} else {
								response.append("461 " + cl.nick + " MODE :Not enough parameters");
								break;
							}
						} else if (modes[i] == 'l' && chnl.user_limit != 0) {
							// remove limit
							chnl.user_limit = 0;
							if (!prefixed) {
								mode_changes.append("-");
								prefixed = true;
							}
							mode_changes.append("l");
						}
					}
				}
				if (mode_changes.length()) {
					string announcement = ":" + cl.fullname + " MODE " + chnlname + " " + mode_changes + "\r\n";
					std::set<string>::iterator it = chnl.users.begin();
					while (it != chnl.users.end()) {
						send(Client::client_list.at(*it).socket, announcement.c_str(), announcement.length(), 0);
						++it;
					}
				}
			}
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
