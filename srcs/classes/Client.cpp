/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 07:51:33 by tjegades          #+#    #+#             */
/*   Updated: 2024/11/06 07:51:34 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

std::map<int, Client &> Client::connections;
std::map<string, Client &> Client::client_list;
string Client::password = "";
string Client::port = "";

Client::Client() {}

Client::Client(const Client&) {}

Client&	Client::operator=(const Client&) {
	return (*this);
}

Client::~Client() {}

Client::Client(int fd, string ip) :
	socket(fd),
	ip_addr(ip),
	nick(""),
	user(""),
	host(""),
	server(""),
	realname(""),
	fullname(""),
	auth(false),
	registered(false) {}

static bool	is_special(char c) {
	if (!(c >= 91 && c <= 96) && !(c >= 123 && c <= 125))
		return (false);
	return (true);
}

bool Client::ready_to_register() const {
	return (this->auth && this->nick.length() && this->host.length() && this->user.length() && !this->registered);
}

bool	Client::is_valid_nick(string nickname) {
	if (nickname.empty() || nickname.size() > 16)
		return (false);
	if (!is_special(nickname[0]) && !std::isalpha(nickname[0]))
		return (false);
	for (size_t i = 1; i < nickname.size(); i++) {
		if (!std::isalpha(nickname[i]) && !std::isdigit(nickname[i]) && !is_special(nickname[i]))
			return (false);
	}
	return (true);
}

bool	Client::is_valid_user(string username) {
	if (username.empty())
		return (false);
	for (unsigned long i = 0; i < username.size(); i++) {
		if (username[i] == '\n' || username[i] == '\r' || username[i] == '\0'
			|| username[i] == '@' || username[i] == ' ')
			return (false);
	}
	return (true);
}
