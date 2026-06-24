#include "LocationConfig.hpp"

LocationConfig::LocationConfig(){
    _autoindex = false;
    _clientMaxBodySize = 1000000;
    _uploadPath = "";
    _redirectCode = 0;
}
LocationConfig::~LocationConfig(){}

std::string LocationConfig::getPath() const{
    return (_path);
}
void LocationConfig::setPath(const std::string& path){
    _path = path;
}

std::string LocationConfig::getRoot() const{
    return (_root);
}

void LocationConfig::setRoot(const std::string& root){
    _root = root;
}

std::vector<std::string>  LocationConfig::getAllowedMethods() const{
    return _allowedMethods;
}
void LocationConfig::addAllowedMethod(const std::string& method)
{
    _allowedMethods.push_back(method);
}
void LocationConfig::setRedirectCode(int code)
{
    _redirectCode = code;
}
void LocationConfig::setRedirectTarget(const std::string& target)
{
    _redirectTarget = target;
}
int LocationConfig::getRedirectCode() const
{
    return _redirectCode;
}
std::string LocationConfig::getRedirectTarget() const
{
    return _redirectTarget;
}
bool LocationConfig::getAutoindex() const{
    return _autoindex;
}
void LocationConfig::setAutoindex(bool autoindex){
    _autoindex = autoindex;
}

const std::string& LocationConfig::getUploadPath() const {
    return _uploadPath;
}
void LocationConfig::setUploadPath(const std::string& uploadPath){
    _uploadPath = uploadPath;
}

size_t LocationConfig::getClientMaxBodySize() const{
    return _clientMaxBodySize;
}
void LocationConfig::setClientMaxBodySize(size_t size){
    _clientMaxBodySize = size;
}

std::map<std::string, std::string> LocationConfig::getCgi() const{
    return _cgi;
}
void    LocationConfig::setCgi(const std::map<std::string, std::string>& cgi){
    _cgi = cgi;
}
std::string LocationConfig::getIndex() const {
    return _index;
}
void LocationConfig::setIndex(const std::string& index){
    _index = index;
}