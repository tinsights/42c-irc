/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Message.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/17 15:21:21 by tjegades          #+#    #+#             */
/*   Updated: 2024/11/06 03:05:31 by xlow             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __MESSAGE_HPP__
# define __MESSAGE_HPP__

#include "irc.hpp"

class Message {
	public:
	Message(string raw);
	// 	/**
	// 	 * TODO:
	// 	 * 	sanity checks (length, not empty, etc.)
	// 	 * 	ends with CRLF?
	// 	 * 	total validity check? max 2 colons? num spaces? etc
	// 	 *
	// 	 *  actually unsure if completely nec to do,
	// 	 *  given IRC protocol expects messages of certain
	// 	 *  format, as long as not undefined or wonky behaviour,
	// 	 * 	but may need to make it netcat-proof
	// 	 *
	// 	 * */

	~Message();

	// eventually private, i think?
	string			cmd;
	string			prefix;
	string			trailing;
	string			params;
	vector<string>		param_list;
	bool			valid;

	private:
	Message();
	Message(const Message&);
	Message&	operator=(const Message&);

};

#endif /* __MESSAGE_HPP__ */
