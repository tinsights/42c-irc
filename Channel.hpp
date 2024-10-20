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
		Channel(string name);
		~Channel();

		// eventually private, lazy to do getters/setters for now:
		string name;

		static std::map<string, std::set<string> > channel_list;
	private:
	
};


#endif /* __CHANNEL_HPP__ */