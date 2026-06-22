#include "ServerConfig.hpp"

ServerConfig::ServerConfig(){
    _clientMaxBodySize = 1000000;
}
ServerConfig::~ServerConfig(){}
const std::vector<int>& ServerConfig::getPorts() const{
    return _ports;
}
void ServerConfig::addPort(int port){
    _ports.push_back(port);
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