/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:21:21 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/21 12:45:26 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __MESSAGE_HPP__
# define __MESSAGE_HPP__

#include "irc.hpp"

/**
 * TODO: 
 * - Message.cpp lol
 * - OCF
*/

class Message {
	public:
	Message(string raw) {
		/**
		 * TODO: 
		 * 	sanity checks (length, not empty, etc.)
		 * 	ends with CRLF?
		 * 	total validity check? max 2 colons? num spaces? etc
		 * 
		 *  actually unsure if completely nec to do,
		 *  given IRC protocol expects messages of certain
		 *  format, as long as not undefined or wonky behaviour.
		 * 
		 * */ 

		/* probably better with a proper ft_split */
		if (raw.find_first_of(':') == 0) {
			prefix = raw.substr(0, raw.find_first_of(' '));
			raw.erase(0, raw.find_first_of(' ') + 1);
		} else {
			prefix = "";
		}
		size_t space_idx = raw.find_first_of(' ');
		if (space_idx != string::npos) {
			cmd = raw.substr(0, raw.find_first_of(' '));
			raw.erase(0, raw.find_first_of(' ') + 1);
		} else {
			cmd = raw;
			raw.erase(raw.begin(), raw.end());
		}
		
		if (raw.length()) {
			size_t trail_index = raw.find_first_of(':');
			params = raw.substr(0, trail_index);
			if (trail_index != string::npos) {
				trailing = raw.substr(trail_index + 1);
			}
		}

		/* can be operator overload instead
			then just YEET msg */
		YEET BOLDRED << setw(20) << "Prefix: " << prefix ENDL;
		YEET BOLDRED << setw(20) << "Cmd: " << cmd ENDL;
		YEET BOLDRED << setw(20) << "Params: " << params ENDL;
		YEET BOLDRED << setw(20) << "Trail: " << trailing ENDL;
		YEET "" ENDL;
	};
	~Message() {};

	// eventually private, i think?
	string	input;
	string	cmd;
	string	prefix;
	string	trailing;
	string	params;
	bool	valid;
	
	private:
	Message();

};

#endif /* __MESSAGE_HPP__ */