#include "Client.hpp"
#include "Server.hpp"


int main()
{
    try
    {
        std::vector<int> ports;
        ports.push_back(8888);
        ports.push_back(8080);
        ports.push_back(9090);

        Server server(ports);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}