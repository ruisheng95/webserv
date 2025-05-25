SRCS = \
$(addprefix src/, Config.cpp main.cpp HTTP.cpp Server.cpp Socket.cpp \
Request.cpp Location.cpp Response.cpp Cgi.cpp Autoindex.cpp utils.cpp)

HEADERS = \
$(addprefix include/, Config.hpp HTTP.hpp Server.hpp Socket.hpp \
Request.hpp Location.hpp Response.hpp Cgi.hpp Autoindex.hpp webserv.hpp)

SRC_DIR = src

OBJ_DIR = obj

OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

NAME = webserv

CC = c++

CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

# %.o: %.cpp
# 	$(CC) $(CFLAGS) -c $< -o $@

${NAME}: ${OBJS} ${HEADERS} Makefile
	${CC} ${CFLAGS} ${OBJS} -o ${NAME}

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: ${NAME} run_cgi_makefile

run_cgi_makefile:
	cd cgi && $(MAKE) -C cgi

clean:
	rm -rf ${OBJ_DIR}

fclean: clean
	rm -f $(NAME)

re: fclean $(NAME)

.PHONY: all clean fclean re run_cgi_makefile
