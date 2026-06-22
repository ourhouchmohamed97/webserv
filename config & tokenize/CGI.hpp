#pragma once

#include <iostream>
#include <map>

class CGI{
public:
    CGI();
    ~CGI();

    std::string execute(const std::string& scriptPath,
                        const std::string& method,
                        const std::string& body);
private:
    void buildEnvr();
    void setupPipe();
    void forkProgress();
};