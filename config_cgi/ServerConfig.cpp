#include "ServerConfig.hpp"

ServerConfig::ServerConfig(){
    _clientMaxBodySize = 0;
    _uploadPath = "";
}
ServerConfig::~ServerConfig(){}
const std::vector<int>& ServerConfig::getPorts() const{
    return _ports;
}
void ServerConfig::addPort(int port){
    for (size_t i = 0; i < _ports.size(); i++)
    {
        if (_ports[i] == port)
            throw std::runtime_error("duplicate port");
    }
    _ports.push_back(port);
}
void ServerConfig::setClientMaxBodySize(int client_max){
    _clientMaxBodySize = client_max;
}
int ServerConfig::getClientMaxBodySize(){
    return _clientMaxBodySize;
}
const std::string& ServerConfig::getUploadPath() const{
    return _uploadPath;
}
void ServerConfig::setUploadPath(const std::string& path){
    _uploadPath = path;
}
std::string ServerConfig::getRoot() const{
    return _root;
}
void ServerConfig::setRoot(const    std::string& root){
    _root = root;
}
std::string ServerConfig::getIndex() const {
    return _index;
}
void ServerConfig::setIndex(const std::string& index){
    _index = index;
}
std::vector<LocationConfig> ServerConfig::getLocations() const{
    return _locations;
}
void ServerConfig::addLocation(const LocationConfig& loc){
    for (size_t i = 0; i < _locations.size(); i++){
    if (_locations[i].getPath() == loc.getPath())
        throw std::runtime_error("duplicate location path");
    }
    _locations.push_back(loc);
}
void ServerConfig::addErrorPage(int code, const std::string& path)
{
    _errorPages[code] = path;
}
std::string ServerConfig::getErrorPage(int code) const
{
    std::map<int, std::string>::const_iterator it = _errorPages.find(code);

    if (it != _errorPages.end())
        return it->second;
    return "";
}
std::map<int, std::string> ServerConfig::getErrorPages() const
{
    return _errorPages;
}