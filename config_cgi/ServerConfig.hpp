#pragma once 

#include <iostream>
#include <vector>
#include <map>
#include "LocationConfig.hpp"

class ServerConfig{
    private:
        std::vector<int>            _ports;
        std::string                 _root; 
        std::string                 _index; 
        std::string                 _uploadPath;
        int                         _clientMaxBodySize;
        std::map<int, std::string>  _errorPages;
        std::vector<LocationConfig> _locations;
    public:
        ServerConfig();
        ~ServerConfig();
        void addPort(int port);
        const std::vector<int>& getPorts() const;
        const std::string& getUploadPath() const;
        void setUploadPath(const std::string& path);
        void    setClientMaxBodySize(int client_max);
        int     getClientMaxBodySize();
        void addErrorPage(int code, const std::string& path);
        std::string getErrorPage(int code) const;
        std::map<int, std::string> getErrorPages() const;
        std::string getRoot() const;
        void        setRoot(const std::string& root);
        void addLocation(const LocationConfig& loc);
        std::vector<LocationConfig> getLocations() const;
        std::string getIndex() const;
        void setIndex(const std::string& index);
};