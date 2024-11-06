# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: tjegades <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/06/30 15:16:11 by tjegades          #+#    #+#              #
#    Updated: 2024/11/06 02:58:54 by xlow             ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv

SRCDIR = srcs/
CLASSDIR = srcs/classes/
INCDIR = includes/ srcs/classes/

SRCS = $(addprefix $(SRCDIR), main.cpp serverutils.cpp cmdutils.cpp)
CLASSES = $(addprefix $(CLASSDIR), Message.cpp Client.cpp Channel.cpp)
INCLUDES = $(addprefix -I, $(INCDIR))

OBJS = $(SRCS:.cpp=.o) $(CLASSES:.cpp=.o)
DEPS = $(SRCS:.cpp=.d) $(CLASSES:.cpp=.d)

CC = c++
CFLAGS = -Wall -Werror -Wextra -g -O3 -std=c++98
CPPFLAGS = -MMD -MP

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS)  -o $(NAME)

%.o: %.cpp
	 $(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean all

debug: CFLAGS += -g -D DEBUG=1
debug: re
	valgrind ./$(NAME) 6667 hitchhiker

-include $(DEPS)

.PHONY: all clean fclean re debug
