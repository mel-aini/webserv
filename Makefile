NAME = webserv

HEADERS = multiplexing/Global.hpp server/Server.hpp server/Client.hpp server/Location.hpp server/Request.hpp server/Response.hpp server/Cgi.hpp public/Colors.hpp \
		parsing/ConfigFile.hpp server/HtmlTemplate.hpp server/Log.hpp

SRC = webserv.cpp server/Location.cpp server/Server.cpp server/UpResponse.cpp parsing/ConfigFile.cpp server/Request.cpp server/Response.cpp server/Cgi.cpp multiplexing/Global.cpp \
		server/Client.cpp server/HtmlTemplate.cpp parsing/parserUtils.cpp parsing/tokenizer.cpp server/Log.cpp

OBJ = $(SRC:.cpp=.o)

FLAGS = -Wall -Wextra -Werror -g -std=c++98 #-fsanitize=address

all : $(NAME)

$(NAME) : $(OBJ)
	@c++ $(FLAGS) $(OBJ) -o $@

%.o : %.cpp $(HEADERS)
	@c++ $(FLAGS) -c $< -o $@

clean :
	@rm -rf $(OBJ)

fclean : clean
	@rm -rf $(NAME)

re : fclean all

run : cleanTraces
	@./$(NAME) webser.conf

cleanTraces:
	@rm -rf Traces/*

.PHONY : clean fclean re run