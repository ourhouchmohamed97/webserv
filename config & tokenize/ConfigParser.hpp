#pragma once

#include <string>
#include <vector>
#include "ServerConfig.hpp"
#include "Token.hpp"

class ConfigParser {
private:
    std::string _filename;
public:
    ConfigParser(const std::string& filename);
    ~ConfigParser();
    std::string readFile() const;
    std::vector<Token> tokenize(const std::string& content);
    std::vector<ServerConfig> parse(const std::vector<Token>& tokens);
};
