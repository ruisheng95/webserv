SRCS = \
Config.cpp main.cpp HTTP.cpp Server.cpp Socket.cpp \
Request.cpp Location.cpp Response.cpp Cgi.cpp Autoindex.cpp

HEADERS = Config.hpp HTTP.hpp Server.hpp Socket.hpp \
Request.hpp Location.hpp Response.hpp Cgi.hpp Autoindex.hpp webserv.hpp

OBJS = $(SRCS:.cpp=.o)

NAME = webserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

${NAME}: ${OBJS} ${HEADERS} Makefile
	${CC} ${CFLAGS} ${OBJS} -o ${NAME}

all: ${NAME} run_cgi_makefile

run_cgi_makefile:
	cd cgi && $(MAKE) -C cgi

clean:
	rm -f ${OBJS}

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re run_cgi_makefile
