#pragma once 

#include <iostream>
#include <vector>
#include <map>
#include "LocationConfig.hpp"

class ServerConfig{
    private:
        int                         _port;
        std::string                 _root; 
        std::string                 _index; 
        size_t                      _clientMaxBodySize;
        std::map<int, std::string>  _errorPages;
        std::vector<LocationConfig> _locations;
    public:
        ServerConfig();
        ~ServerConfig();
        int     getPort() const;
        void    setPort(int port);
        void addLocation(const LocationConfig& loc);
        std::vector<LocationConfig> getLocations() const;
        std::string getIndex() const;
        void setIndex(const std::string& index);
};