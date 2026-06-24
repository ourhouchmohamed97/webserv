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
    for (size_t i = 0; i < content.size(); i++){
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
    bool rootSet = false;
    bool indexSet = false;
    bool uploadPathSet = false;
    bool locRootSet = false;
    bool locIndexSet = false;
    bool locUploadSet = false;
    bool locAutoindexSet = false;
    bool locRedirectSet = false;
    bool locMethodsSet = false;

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
            if (tokens[i].value == "listen"){
                i++;
            if (i >= tokens.size())
                throw std::runtime_error("listen: missing value");
            if (tokens[i].value == ";")
                throw std::runtime_error("listen: missing port");
            while (i < tokens.size() && tokens[i].value != ";"){
                if (!isNumber(tokens[i].value))
                    throw std::runtime_error("listen: invalid port");
                int port = std::atoi(tokens[i].value.c_str());
                if (port < 1 || port > 65535)
                    throw std::runtime_error("listen: port out of range");
                server.addPort(port);
               i++;
            }
            if (i >= tokens.size() || tokens[i].value != ";")
                throw std::runtime_error("listen: missing ';'");
            i++;
            }
            else if (tokens[i].value == "root")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("root: missing value");
                if (rootSet)
                    throw std::runtime_error("duplicate root directive");
                rootSet = true;
                server.setRoot(tokens[i].value);
                i++;

                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after root");
                i++;
            }
            else if (tokens[i].value == "upload_path")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("upload_path missing value");
                if(uploadPathSet)
                    throw std::runtime_error("duplicate upload path");
                uploadPathSet = true;
                server.setUploadPath(tokens[i].value);
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after upload_path");
                i++;
            }
            else if (tokens[i].value == "index")
            {
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("index: missing value");
                if (indexSet)
                    throw std::runtime_error("duplicate index");
                indexSet = true;
                server.setIndex(tokens[i].value);
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';' after index");
                i++;
            }
            else if (tokens[i].value == "client_max_body_size"){
                i++;
                if (i >= tokens.size() || !isNumber(tokens[i].value))
                    throw std::runtime_error("invalid client_max_body_size");
                server.setClientMaxBodySize(std::atoi(tokens[i].value.c_str()));
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';'");
                    i++;
            }
            else if (tokens[i].value == "error_page"){
                i++;
                int code = std::atoi(tokens[i].value.c_str());
                i++;
                if (i >= tokens.size())
                    throw std::runtime_error("error_page missing path");
                    server.addErrorPage(code, tokens[i].value);
                i++;
                if (i >= tokens.size() || tokens[i].value != ";")
                    throw std::runtime_error("missing ';'");
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
                    if (tokens[i].value == "root"){
                        if (locRootSet)
                            throw std::runtime_error("duplicate root in location");
                        locRootSet = true;
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("location root: missing value");
                        loc.setRoot(tokens[i].value);
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after location root");
                        i++;
                    }
                    else if (tokens[i].value == "redirect"){
                        if (locRedirectSet)
                            throw std::runtime_error("duplicate redirect in location");
                        locRedirectSet = true;
                        i++;
                        if (i >= tokens.size() || !isNumber(tokens[i].value))
                            throw std::runtime_error("redirect: missing code");
                        loc.setRedirectCode(std::atoi(tokens[i].value.c_str()));
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("redirect: missing target");
                        loc.setRedirectTarget(tokens[i].value);
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after redirect");
                        i++;
                    }
                    else if (tokens[i].value == "client_max_body_size"){
                        i++;
                        if (i >= tokens.size() || !isNumber(tokens[i].value))
                            throw std::runtime_error("invalid client_max_body_size");
                        server.setClientMaxBodySize(std::atoi(tokens[i].value.c_str()));
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';'");
                        i++;
                    }
                    else if (tokens[i].value == "autoindex"){
                        if (locAutoindexSet)
                            throw std::runtime_error("duplicate autoindex in location");
                        locAutoindexSet = true;
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("autoindex missing value");
                        if (tokens[i].value != "on" && tokens[i].value != "off")
                            throw std::runtime_error("autoindex must be on or off");
                        loc.setAutoindex(tokens[i].value == "on");
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';'");
                        i++;
                    }
                    else if (tokens[i].value == "allow_methods"){
                        if (locMethodsSet)
                            throw std::runtime_error("duplicate allow_methods in location");
                        locMethodsSet = true;
                        i++;
                        if (i < tokens.size() && tokens[i].value == ";")
                            throw std::runtime_error("allow_methods: missing methods");
                        while (i < tokens.size() && tokens[i].value != ";"){
                            if (tokens[i].value != "GET"
                                && tokens[i].value != "POST"
                                && tokens[i].value != "DELETE")
                                    throw std::runtime_error("invalid method: " + tokens[i].value);
                                loc.addAllowedMethod(tokens[i].value);
                            i++;
                        }
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after methods");
                        i++;
                    }
                    else if (tokens[i].value == "upload_path"){
                        if (locUploadSet)
                            throw std::runtime_error("duplicate upload_path in location");
                        locUploadSet = true;
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("upload_path missing value");
                        loc.setUploadPath(tokens[i].value);
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after upload_path");
                        i++;
                    }
                    else if (tokens[i].value == "index"){
                        if (locIndexSet)
                            throw std::runtime_error("duplicate index in location");
                        locIndexSet = true;
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("location index: missing value");
                        loc.setIndex(tokens[i].value);
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after location index");
                        i++;
                    }
                    else if (tokens[i].value == "cgi"){
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("cgi: missing extension");
                        std::string ext = tokens[i].value;
                        i++;
                        if (i >= tokens.size())
                            throw std::runtime_error("cgi: missing interpreter");
                        std::string interpreter = tokens[i].value;
                        i++;
                        if (i >= tokens.size() || tokens[i].value != ";")
                            throw std::runtime_error("missing ';' after cgi");
                        std::map<std::string, std::string> cgi = loc.getCgi();
                        cgi[ext] = interpreter;
                        loc.setCgi(cgi);
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