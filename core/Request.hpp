#pragma once

#include <iostream>
#include <map>
#include <sstream>
class Request
{
    public:
        std::string method;
        std::string version;
        std::string path;

        std::map<std::string, std::string> headers;
        
        void   parse(const std::string &raw_request);


};

