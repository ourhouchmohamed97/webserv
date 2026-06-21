NAME	 = webserv
CXX		 = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRC		 = 	main.cpp  Server.cpp  \
			Client.cpp  Request.cpp
			

HEADER = Server.hpp Client.hpp Request.hpp 

OBJ		 = $(SRC:.cpp=.o)

all: $(NAME)


$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean: 
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

