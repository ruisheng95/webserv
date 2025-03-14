SRCS = \
cgi.cpp

OBJS = $(SRCS:.c=.o)

NAME = upload.cgi

CC = c++

CFLAGS = -Wall -Wextra -Werror

${NAME}: ${SRCS}
	${CC} ${CFLAGS} ${SRCS} -o ${NAME}

all : ${NAME}

clean :
	rm -rf ${NAME}

fclean : clean
	rm -rf $(NAME)

re : fclean $(NAME)

.PHONY : all clean flcean re bonus