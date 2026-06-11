#include "ConfigParser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
int main()
{
    try
    {
        ConfigParser parser("webserv.conf");
        std::string content = parser.readFile();
        std::vector<Token> tokens = parser.tokenize(content);
        std::cout << content << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}