NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror
RM = rm -f

HEADERS = \
	config_cgi/cgi.hpp \
	config_cgi/ConfigParser.hpp \
	config_cgi/LocationConfig.hpp \
	config_cgi/ServerConfig.hpp \
	config_cgi/Token.hpp \
	core/Client.hpp \
	core/Request.hpp \
	core/Server.hpp \
	request_response/AutoIndex.hpp \
	request_response/ChunkDecoder.hpp \
	request_response/ErrorPageFactory.hpp \
	request_response/HttpServer.hpp \
	request_response/HttpUtils.hpp \
	request_response/MimeTypeHelper.hpp \
	request_response/MultipartParser.hpp \
	request_response/PathResolver.hpp \
	request_response/RequestParser.hpp \
	request_response/RouteMatcher.hpp \
	request_response/StaticFileServer.hpp


SRC = \
	config_cgi/cgi.cpp \
	config_cgi/ConfigParser.cpp \
	config_cgi/LocationConfig.cpp \
	config_cgi/ServerConfig.cpp \
	config_cgi/Token.cpp \
	core/Client.cpp \
	core/Request.cpp \
	core/Server.cpp \
	main.cpp \

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all
