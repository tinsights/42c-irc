#include "Channel.hpp"

Channel::Channel() {}

Channel::Channel(const Channel&) {}

Channel&	Channel::operator=(const Channel&) {
	return (*this);
}

Channel::~Channel() {}

Channel::Channel(string name) :
name(name),
opers(),
users(),
passwd(""),
topic(""),
invite_only(false),
topic_protected(true),
passwd_protected(false) {}

bool	Channel::is_valid_channel(string channel) {
	if (channel.empty() || channel.size() > 50)
		return (false);
	if (channel[0] != '&' && channel[0] != '#'
		&& channel[0] != '+' && channel[0] != '!')
		return (false);
	for (unsigned long i = 1; i < channel.size(); i++) {
		if (channel[i] == ' ' || channel[i] == 7 || channel[i] == ',')
			return (false);
	}
	return (true);
}

bool	Channel::is_valid_pass(string password) {
	if (password.empty() || password.size() > 23)
		return (false);
	for (unsigned long i = 0; i < password.size(); i++) {
		if (password[i] == '\0' || password[i] == '\r'
			|| std::isspace(password[i]))
			return (false);
	}
	return (true);
}