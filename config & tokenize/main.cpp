#include "CGI.hpp"
#include <iostream>

int main()
{
    CGI cgi;

    try
    {
        std::string result = cgi.execute(
            "./test.py",
            "GET",
            ""
        );

        std::cout << "CGI OUTPUT:\n";
        std::cout << result << std::endl;
    }
    catch (std::exception &e)
    {
        std::cout << "ERROR: " << e.what() << std::endl;
    }

    return 0;
}