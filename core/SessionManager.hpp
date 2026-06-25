#pragma once 

#include <map>
#include <string>
#include "../request_response/HttpUtils.hpp"
#include <ctime>
#include <sstream>
#include <cstdlib>

class SessionManager
{
    private:
        static std::map<std::string, int> _sessions;
        
        std::string generateId();
        std::string get_cookies_val(const std::string &cookies_header, const std::string &key);


    public:
        HttpResponse handle(const HttpRequest &req);
};

