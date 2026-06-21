#include "Client.hpp"

Client::Client() : fd(-1), state(CLOSED){}

Client::Client(int fd) : fd(fd), state(READING){}

