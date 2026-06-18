#include "ConfigParser.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

ConfigParser::ConfigParser(const std::string& filename){
    _filename = filename;
}
ConfigParser::~ConfigParser(){}

std::string ConfigParser::readFile() const{
    std::ifstream file(_filename.c_str());
    if (!file.is_open())
        throw std::runtime_error("Could not open the file");
    std::stringstream buffer;
    buffer << file.rdbuf();
    return (buffer.str());
}
std::vector<Token> ConfigParser::tokenize(const std::string& content){
    std::vector<Token> tokens;
    std::string word;
    for (int i = 0; i < content.size(); i++){
        char c = content[i];
        if (c == '{')
        {
            if (!word.empty()){
                tokens.push_back(Token(WORD, word));
                word.clear();
            }
            tokens.push_back(Token(OPEN_BRACE, "{"));
        }
        else if (c == '}'){
            if (!word.empty()){
                tokens.push_back(Token(WORD, word));
                word.clear();
            }
            tokens.push_back(Token(CLOSE_BRACE, "}"));
        }
        else if (c == ';') {
            if (!word.empty()){
                tokens.push_back(Token(WORD, word));
                word.clear();
            }
                tokens.push_back(Token(SEMICOLON, ";"));
        }
        else if (isspace(c)){
            if (!word.empty()){
                tokens.push_back(Token(WORD,word));
                word.clear();
            }
        }
        else
            word += c;
    }
    if (!word.empty())
        tokens.push_back(Token(WORD, word));
    return (tokens);
}

std::vector<ServerConfig> ConfigParser::parse(const std::vector<Token>& tokens)
{
    std::vector<ServerConfig> servers;
    size_t i = 0;

    while (i < tokens.size())
    {
        if (tokens[i].value != "server")
            throw std::runtime_error("expected 'server' keyword");
        i++;
        if (i >= tokens.size() || tokens[i].value != "{")
            throw std::runtime_error("expected '{' after server");
        i++;
        ServerConfig server;
        while (i < tokens.size() && tokens[i].value != "}")
        {
            if (tokens[i].value == "listen")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("listen: missing port value");
                if (!isNumber(tokens[i].value))
                    throw std::runtime_error("listen: invalid port");
                int port = std::atoi(tokens[i].value.c_str());
                if (port < 1 || port > 65535)
                    throw std::runtime_error("listen: port out of range");
                server.setPort(port);
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after listen");
                i++;
            }
            else if (tokens[i].value == "root")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("root: missing value");
                server.setRoot(tokens[i].value);
                i++;

                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after root");
                i++;
            }
            else if (tokens[i].value == "index")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("index: missing value");
                server.setIndex(tokens[i].value);
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after index");
                i++;
            }
            else if (tokens[i].value == "location")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("location: missing path");
                std::string path = tokens[i].value;
                i++;
                if (i >= tokens.size() || tokens[i].value != "{")
                    throw std::runtime_error("expected '{' after location path");
                i++;
                LocationConfig loc;
                loc.setPath(path);
                while (i < tokens.size() && tokens[i].value != "}")
                {
                    if (tokens[i].value == "root")
                    {
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("location root: missing value");

                        loc.setRoot(tokens[i].value);
                        i++;

                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after location root");
                        i++;
                    }
                    else if (tokens[i].value == "index")
                    {
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("location index: missing value");

                        loc.setIndex(tokens[i].value);
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after location index");
                        i++;
                    }
                    else
                        throw std::runtime_error("unknown directive in location block: " + tokens[i].value);
                }
                if (i >= tokens.size())
                    throw std::runtime_error("unclosed location block (missing '}')");
                i++;
                server.addLocation(loc);
            }
            else
                throw std::runtime_error("unknown directive in server block: " + tokens[i].value);
        }
        if (i >= tokens.size())
            throw std::runtime_error("unclosed server block (missing '}')");
        i++;
        servers.push_back(server);
    }
    return servers;
}

bool ConfigParser::isNumber(const std::string& str) const{
    if (str.empty())
        return false;
    for (size_t i = 0; i < str.size(); i++){
        if (!std::isdigit(str[i]))
            return false;
    }
    return true;
}