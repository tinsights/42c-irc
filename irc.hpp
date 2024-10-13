/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   irc.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/03 12:04:27 by tjegades          #+#    #+#             */
/*   Updated: 2024/10/03 12:04:27 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#ifndef __IRC_HPP__
#define __IRC_HPP__

# include <sys/socket.h> // cpp ver?
# include <netdb.h> // cpp ver?
# include <iostream>
# include <unistd.h>
# include <string>
# include <cstring>
# include <poll.h>
# include "colours.hpp"

using std::cout;
using std::clog;
using std::endl;
using std::string;

# ifndef DEBUG
#  define DEBUG 0
# endif

# define YEET if (DEBUG) clog << BOLDBLACK <<
# define ENDL << RESET << endl

typedef struct addrinfo ai;

#endif /* __IRC_HPP__ */