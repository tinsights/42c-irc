#include "Client.hpp"

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

bool	Client::is_valid_nick(string nickname) {
	if (nickname.empty() || nickname.size() > 8)
		return (false);
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