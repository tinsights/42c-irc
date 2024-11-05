/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tinaes <tinaes@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/20 14:18:00 by tinaes            #+#    #+#             */
/*   Updated: 2024/10/20 14:35:29 by tinaes           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __CHANNEL_HPP__
# define __CHANNEL_HPP__

#include "irc.hpp"

class Channel {
	public:
		Channel(string name) 
			: name(name), opers(), users(), invited(), passwd(""), topic(""), invite_only(false), topic_protected(true), passwd_protected(false), user_limit(0) {};
		~Channel() {};

		// eventually private, lazy to do getters/setters for now:
		string name;			//chn name. todo: validation

		std::set<string> opers; // nicks of users who are operator
		std::set<string> users;
		std::set<string> invited; // nicks of users who are invited	

		/**
		 * TODO: all of the below
		*/
		string passwd;
		string topic;

		bool 	invite_only;
		bool	topic_protected;
		bool	passwd_protected;
		size_t	user_limit; // default to 0 instead of toggling switch

		static std::map<string, Channel & > channel_list;
	private:
		Channel() {};
};


#endif /* __CHANNEL_HPP__ */