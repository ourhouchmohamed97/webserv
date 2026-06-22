#pragma once

#include <iostream>
#include <map>
#include <string>
#include <sstream>
class CGI{
public:
    CGI();
    ~CGI();

    std::string execute(const std::string& scriptPath,
                        const std::string& method,
                        const std::string& body);
    template <typename T>
    std::string to_string(const T& value) {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }
private:
    void buildEnvr();
    void setupPipe();
    void forkProgress();
};