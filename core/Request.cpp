#include "Request.hpp"


std::string trim(std::string &str)
{
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void    Request::parse(const std::string &raw_request)
{
    std::istringstream str(raw_request);
    std::string line;


    if (getline(str, line))
    {
        std::istringstream first_line(line);
        first_line >> method >> path >> version;
    }

    while(getline(str, line))
    {
        line = trim(line);
        if (line.empty())
            break;
        size_t pos = line.find(":");
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            headers[trim(key)] = trim(value);
        }
    }
}