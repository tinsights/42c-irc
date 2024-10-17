/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:17:36 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/17 15:17:36 by tjegades         ###   ########.fr       */
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
		Client(int fd, string ip) : socket(fd), ip_addr(ip), nick(""), user(""), host(""), server(""), realname(""), fullname(""), remainder(""), auth(false), registered(false) {};
		// TODO: OCF format etc
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

#endif /* __CLIENT_HPP__ */