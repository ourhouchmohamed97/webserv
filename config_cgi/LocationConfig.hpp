#pragma once

#include <string>
#include <vector>
#include <map>

class LocationConfig
{
private:
    std::string                         _path;
    std::string                         _root;
    int                                 _redirectCode;
    std::string                         _redirectTarget;
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
    void setRedirectCode(int code);
    void setRedirectTarget(const std::string& target);
    int getRedirectCode() const;
    std::string getRedirectTarget() const;
    std::string getRoot() const;
    void setRoot(const std::string& root);
    const std::string& getUploadPath() const;
    void setUploadPath(const std::string& path);
    std::vector<std::string> getAllowedMethods() const;
    void addAllowedMethod(const std::string& method);
    bool getAutoindex() const;
    void setAutoindex(bool autoindex);
    size_t getClientMaxBodySize() const;
    void    setClientMaxBodySize(size_t size);
    std::map<std::string, std::string> getCgi() const;
    void setCgi(const std::map<std::string, std::string>& Cgi);

    std::string getIndex() const;
    void setIndex(const std::string& index);
};