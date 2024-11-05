#include "Message.hpp"

Message::Message() {}

Message::Message(const Message&) {}

Message&	Message::operator=(const Message&) {
	return (*this);
}

Message::~Message() {}

static void	get_prefix(string &raw, string &prefix) {
	if (raw[0] == ':') {
		if (std::isspace(raw[1]))
			throw (std::invalid_argument("Whitespace after prefix colon"));
		size_t	space_idx = raw.find_first_of(' ');
		while (raw[space_idx] == ' ')
			space_idx++;
		if (space_idx == raw.length() - 1)
			throw (std::invalid_argument("No cmd found after prefix"));
		prefix = raw.substr(1, raw.find_first_of(' ') - 1);
		raw.erase(0, space_idx + 1);
	}
}

static void	get_cmd(string &raw, string &cmd) {
	size_t space_idx = raw.find_first_of(' ');
	if (space_idx != string::npos) {
		cmd = raw.substr(0, space_idx);
		while (raw[space_idx] == ' ')
			space_idx++;
		raw.erase(0, space_idx);
	}
	else {
		cmd = raw;
		raw.clear();
	}
}

static void	get_params(string &raw, string &params, string &trailing) {
	if (raw.length()) {
		size_t	trailing_sep = raw.find(" :");
		if (trailing_sep == string::npos)
			params = raw;
		else {
			size_t space_idx = trailing_sep;
			while (space_idx > 0 && raw[space_idx] == ' ')
				space_idx--;
			params = raw.substr(0, space_idx + 1);
			raw.erase(0, trailing_sep + 2);
			trailing = raw;
		}
		raw.clear();
	}
}

Message::Message(string &raw) {
	if (raw.length() >= 510)
		throw (std::invalid_argument("Message exceeds 510 characters"));
	get_prefix(raw, this->prefix);
	get_cmd(raw, this->cmd);
	get_params(raw, this->params, this->trailing);
	YEET BOLDRED << setw(20) << "Prefix: |" << prefix << "|" ENDL;
	YEET BOLDRED << setw(20) << "Cmd: |" << cmd << "|" ENDL;
	YEET BOLDRED << setw(20) << "Params: |" << params << "|" ENDL;
	YEET BOLDRED << setw(20) << "Trail: |" << trailing << "|" ENDL;
	YEET "" ENDL;
}