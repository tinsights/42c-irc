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
