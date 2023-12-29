NAME = webserv

HEADERS = multiplexing/Global.hpp server/Server.hpp server/Client.hpp server/Location.hpp server/Request.hpp server/Response.hpp public/Colors.hpp \
		parsing/ConfigFile.hpp

SRC = webserv.cpp server/Location.cpp server/Server.cpp parsing/ConfigFile.cpp server/Request.cpp server/Response.cpp multiplexing/Global.cpp server/Client.cpp

OBJ = $(SRC:.cpp=.o)

FLAGS = -Wall -Wextra -Werror -g -std=c++98 

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

run : all clean
	@./$(NAME) webserv.conf

.PHONY : clean fclean re run