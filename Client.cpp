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
auth(false),
registered(false) {}