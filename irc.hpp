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

/* sockets, poll, bind, etc*/
# include <sys/socket.h> // cpp ver?
# include <netdb.h> // cpp ver?
# include <poll.h>
# include <unistd.h> // read

/* sigint */
# include <csignal> 

/* containers */
# include <map>
# include <vector>
# include <set>
# include <algorithm>

/* system i/o, strings, etc */
# include <iostream>
# include <iomanip>
# include <sstream>
# include <string>
# include <cstring> // um, damn i should really keep track
# include "colours.hpp"

using std::cout;
using std::clog;
using std::endl;

using std::setw;

using std::string;

/* Forward declaration.. can be removed later w proper .cpp class files*/
class Client;
class Message;

# ifndef DEBUG
#  define DEBUG 0
# endif

# define YEET if (DEBUG) clog << BOLDBLACK <<
# define ENDL << RESET << endl

typedef struct addrinfo ai;

#endif /* __IRC_HPP__ */