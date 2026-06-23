#include "ServerConfig.hpp"

ServerConfig::ServerConfig(){
    _clientMaxBodySize = 1000000;
    _uploadPath = "";
}
ServerConfig::~ServerConfig(){}
const std::vector<int>& ServerConfig::getPorts() const{
    return _ports;
}
void ServerConfig::addPort(int port){
    _ports.push_back(port);
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
    _locations.push_back(loc);
}