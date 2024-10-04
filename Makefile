# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tjegades <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/30 15:16:11 by tjegades          #+#    #+#              #
#    Updated: 2024/08/06 10:16:36 by tjegades         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRCS = main.cpp utils.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

CC = c++
CFLAGS = -Wall -Werror -Wextra -g -std=c++98
CPPFLAGS = -MMD -MP

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp
	 $(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CFLAGS += -g -D DEBUG=1
debug: re

-include $(DEPS)

.PHONY: all clean fclean re debug
