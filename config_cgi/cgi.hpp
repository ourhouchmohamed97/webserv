#pragma once

#include <iostream>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include <vector>
#include <sys/wait.h>
#include <sstream>

class CGI{
public:

    std::string execute(const std::string& interpreter,
                        const std::string& scriptPath,
                        const std::string& method,
                        const std::string& body,
                        const std::map<std::string, std::string>& headers);
private:
    void buildEnvr();
    void setupPipe();
    void forkProgress();
};