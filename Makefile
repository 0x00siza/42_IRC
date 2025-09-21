CPP = c++

CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

NAME = ircserv

SRC = $(wildcard src/*.cpp)

HEADER = $(wildcard includes/*.hpp)

OBJ = $(SRC:.cpp=.o)

# Colors
PURPLE = \033[0;35m
NC = \033[0m # No Color

all: $(NAME)

$(NAME): $(OBJ)
	@$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJ)
	@echo "$(PURPLE)Build completed successfully!$(NC)"

%.o: %.cpp $(HEADER)
	@$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	@echo "$(PURPLE)Cleaning...$(NC)"
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all