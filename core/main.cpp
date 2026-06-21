#include "Client.hpp"
#include "Server.hpp"


int main()
{
    try
    {
        Server server(8888);
        server.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}