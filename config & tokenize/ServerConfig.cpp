#include "ServerConfig.hpp"

ServerConfig::ServerConfig(){
    _port = 80;
    _clientMaxBodySize = 1000000;
}
ServerConfig::~ServerConfig(){}
int ServerConfig::getPort() const{
    return _port;
}
void ServerConfig::setPort(int port){
    _port = port;
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