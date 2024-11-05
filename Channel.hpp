/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 14:18:00 by tinaes            #+#    #+#             */
/*   Updated: 2024/11/06 03:03:54 by xlow             ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __CHANNEL_HPP__
# define __CHANNEL_HPP__

#include "irc.hpp"

class Channel {
	public:
		Channel(string name);
		~Channel();

		static bool	is_valid_channel(string channel);
		static bool	is_valid_pass(string password);

		// eventually private, lazy to do getters/setters for now:
		string name;			//chn name. todo: validation

		std::set<string> opers; // nicks of users who are operator
		std::set<string> users;

		/**
		 * TODO: all of the below
		*/
		string passwd;
		string topic;

		bool 	invite_only;
		bool	topic_protected;
		bool	passwd_protected;
		size_t	user_limit; // default to high number instead of toggling switch

		static std::map<string, Channel & > channel_list;
	private:
		Channel();
		Channel(const Channel&);
		Channel& operator=(const Channel&);
};


#endif /* __CHANNEL_HPP__ */
