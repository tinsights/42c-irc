# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tjegades <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/30 15:16:11 by tjegades          #+#    #+#              #
#    Updated: 2024/11/05 20:57:05 by xlow             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRCS = main.cpp serverutils.cpp cmdutils.cpp Message.cpp
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
	valgrind ./$(NAME)

-include $(DEPS)

.PHONY: all clean fclean re debug
