NAME = webserv

HEADERS = $(wildcard */*.hpp) $(wildcard *.hpp)

SRC = $(wildcard */*.cpp) $(wildcard *.cpp)

OBJ_DIR = obj
OBJ = $(addprefix $(OBJ_DIR)/, $(SRC:.cpp=.o))

FLAGS = -Wall -Wextra -Werror -std=c++98 -fsanitize=address

all: $(NAME)

$(NAME) : $(OBJ)
	@c++ $(FLAGS) $(OBJ) -o $@
	@echo "\033[32m$@ created\033[0m"

$(OBJ_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(dir $@)
	@c++ $(FLAGS) -c $< -o $@

clean:
	@rm -rf $(OBJ_DIR)
	@echo "\033[31m$(OBJ_DIR) deleted\033[0m"

fclean: clean
	@rm -rf $(NAME)
	@echo "\033[31m$(NAME) deleted\033[0m"

re: fclean all

run: cleanTraces
	@./$(NAME) webser.conf

cleanTraces:
	@rm -rf Traces/*

.PHONY: all clean fclean re run cleanTraces
