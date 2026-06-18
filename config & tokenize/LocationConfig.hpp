#pragma once

#include <string>
#include <vector>
#include <map>

class LocationConfig
{
private:
    std::string                         _path;
    std::string                         _root;
    std::string                         _index;
    std::vector<std::string>            _allowedMethods;
    bool                                _autoindex;
    std::string                         _uploadPath;
    size_t                              _clientMaxBodySize;
    std::map<std::string, std::string>  _cgi;
public:
    LocationConfig();
    ~LocationConfig();

    std::string getPath() const;
    void setPath(const std::string& path);
    std::string getRoot() const;
    void setRoot(const std::string& root);

    std::vector<std::string> getAllowedMethods() const;
    void setAllowedMethods(const std::vector<std::string>& methods);

    bool getAutoindex() const;
    void setAutoindex(bool autoindex);

    std::string getUploadPath() const;
    void setUploadPath(const std::string& uploadPath);

    size_t getClientMaxBodySize() const;
    void    setClientMaxBodySize(size_t size);

    std::map<std::string, std::string> getCgi() const;
    void setCgi(const std::map<std::string, std::string>& Cgi);
    
    std::string getIndex() const;
    void setIndex(const std::string& index);
};