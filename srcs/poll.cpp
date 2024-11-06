/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   poll.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tjegades <tjegades@student.42singapor      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/30 18:16:08 by tjegades          #+#    #+#             */
/*   Updated: 2024/09/30 18:16:09 by tjegades         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <poll.h>
#include <iostream>

int main (void) {
	struct pollfd pfds[1];

	pfds[0].fd = 0;
	pfds[0].events = POLLIN;

	std::cout << "hit return or wait 2.5 seconds" << std::endl;

	int num_events = poll(pfds, 1, 2500);

	if (!num_events) {
		std::cout << "timeout" << std::endl;
	} else {
		int pollin_happened = pfds[0].revents & POLLIN;
		if (pollin_happened)
			std::cout << "fd ready to read" << std::endl;
		else
			std::cerr << "oops? " << pfds[0].revents;
	}

}