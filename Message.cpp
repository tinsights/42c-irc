/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/06 07:51:53 by tjegades          #+#    #+#             */
/*   Updated: 2024/11/06 07:51:54 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Message.hpp"

Message::Message() {}

Message::Message(const Message&) {}

Message&	Message::operator=(const Message&) {
	return (*this);
}

Message::~Message() {}

static void	get_prefix(string &raw, string &prefix) {
	if (raw[0] == ':') {
		if (raw.size() == 1)
			throw (std::invalid_argument("Empty message after colon"));
		if (std::isspace(raw[1]))
			throw (std::invalid_argument("Whitespace after prefix colon"));
		size_t	space_idx = raw.find_first_of(' ');
		while (raw[space_idx] == ' ')
			space_idx++;
		if (space_idx == raw.length() - 1)
			throw (std::invalid_argument("No cmd found after prefix"));
		prefix = raw.substr(1, raw.find_first_of(' ') - 1);
		raw.erase(0, space_idx);
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
		if (trailing_sep == string::npos) {
			size_t space_idx = raw.size() - 1;
			while (space_idx && raw[space_idx] == ' ')
				space_idx--;
			params = raw.substr(0, space_idx + 1);
			raw.clear();
		}
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

static void	count_params(string &params, string& trailing, vector<string>& param_list) {
	int		available_params = 15;
	int		params_len = params.length();
	int		i = 0;
	string	param;
	if (!trailing.empty())
		available_params--;
	while (i < params_len) {
		available_params--;
		if (available_params < 0)
			throw (std::invalid_argument("Received more than 15 params"));
		while (i < params_len && params[i] != ' ') {
			param.push_back(params[i]);
			i++;
		}
		while (i < params_len && params[i] == ' ')
			i++;
		param_list.push_back(param);
		param.clear();
	}
	YEET BOLDRED << setw(20) << "Param count: " << 15 - available_params ENDL;
}

Message::Message(string raw) :
cmd(""),
prefix(""),
trailing(""),
params("")
{
	this->valid = true;
	try {
		if (raw.length() >= 510)
			throw (std::invalid_argument("Message exceeds 510 characters"));
		get_prefix(raw, this->prefix);
		get_cmd(raw, this->cmd);
		get_params(raw, this->params, this->trailing);
		count_params(this->params, this->trailing, this->param_list);
	}
	catch (std::invalid_argument& e) {
		YEET BOLDRED << e.what() ENDL;
		this->valid = false;
	}
	YEET BOLDRED << setw(20) << "Prefix: |" << prefix << "|" ENDL;
	YEET BOLDRED << setw(20) << "Cmd: |" << cmd << "|" ENDL;
	YEET BOLDRED << setw(20) << "Params: |" << params << "|" ENDL;
	YEET BOLDRED << setw(20) << "Trail: |" << trailing << "|" ENDL;
	YEET BOLDRED << setw(20) << "Param list: " ENDL;
	for (unsigned long i = 0; i < this->param_list.size(); i++) {
		YEET BOLDRED << setw(20) << param_list[i] ENDL;
	}
	YEET "" ENDL;
}
