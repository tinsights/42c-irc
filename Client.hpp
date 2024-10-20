/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:17:36 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/20 14:25:55 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __CLIENT_HPP__
# define __CLIENT_HPP__
# include "irc.hpp"

/**
 * TODO: 
 * - Client.cpp
 * - OCF
*/
class Client {
	public:
		// string ip unnec. was playing.
		Client(int fd, string ip) : socket(fd), ip_addr(ip), nick(""), user(""), host(""), server(""), realname(""), fullname(""), remainder(""), auth(false), registered(false) {};
		// TODO: OCF format etc
		~Client() {};

		// eventually private, with getters and setters etc:
		int 	socket;
		string	ip_addr;	// unnec.
		string	nick;		// uuid. impt!
		string	user;		// username, unsure of importance.
		string	host;		// client hostname i.e. $HOST
		string	server;		// tbh unnec, since we are always the server. can be refactored away
		string	realname;	// realname as in Xian Feng Low
		string	fullname;	// as in <nick>!<host>@<server>
		string	remainder;	// if transmission did not end with CRLF, e.g. netcat with ctrl+d.

		bool	auth;		// if password provided was correct
		bool	registered;	// if successfully registered with unique NICK and full USER details

		std::set<string> joined_channels;

		static std::map<string, Client &> client_list;			// currently "global"
		static std::map<string, std::set<string> > channels;	// likely will be refactored

	private:
		Client() {};
		
};

#endif /* __CLIENT_HPP__ */