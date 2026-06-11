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
        if (!word.empty())
            tokens.push_back(Token(WORD, word));
    }
    return (tokens);
}

std::vector<ServerConfig> ConfigParser::parse(const std::vector<Token>& tokens){
    std::vector<ServerConfig> servers;
    size_t  i = 0;
    while (i < tokens.size()){
        if (tokens[i].value == "server"){
            i++;
            ServerConfig server;
            if (i < tokens.size() && tokens[i].value == "{")
                i++;
            while (i < tokens.size() && tokens[i].value != "}"){
                if (tokens[i].value == "listen"){
                    i++;
                    if (i < tokens.size()){
                        int port = std::atoi(tokens[i].value.c_str());
                        server.setPort(port);
                    }
                }
                else if (tokens[i].value == "location"){
                    i++;
                    std::string path = tokens[i].value;
                    i++;
                    LocationConfig loc;
                    loc.setPath(path);
                    if (i < tokens.size() && tokens[i].value == "{")
                        i++;
                    while (i < tokens.size() && tokens[i].value != "}"){
                        if (tokens[i].value == "root"){
                            i++;
                            if (i < tokens.size())
                                loc.setRoot(tokens[i].value);
                        }
                        else if (tokens[i].value == "index"){
                            i++;
                            if (i < tokens.size())
                            loc.setIndex(tokens[i].value);
                        }
                        i++;
                    }
                    server.addLocation(loc);
                }
                i++;
            }
            servers.push_back(server);
        }
        i++;
    }
    return servers;
}