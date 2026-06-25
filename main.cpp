#include <iostream>
#include <vector>
#include <string>
#include "core/Server.hpp"
#include "config_cgi/ConfigParser.hpp"
#include "config_cgi/Token.hpp"
#include "config_cgi/ServerConfig.hpp"

int main(int argc, char* argv[])
{
    // 1. Determine config path from arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " [config_file_path]" << std::endl;
        return 1;
    }
    std::string configPath = argv[1];

    try
    {
        std::cout << "Loading configuration file: " << configPath << std::endl;

        // 2. Initialize and run the team's tokenizing configuration system
        ConfigParser parser(configPath);
        std::string content = parser.readFile();
        std::vector<Token> tokens = parser.tokenize(content);
        std::vector<ServerConfig> servers = parser.parse(tokens);

        if (servers.empty()) {
            throw std::runtime_error("Error: Configuration file contains no server blocks.");
        }

        // 3. Automatically extract listening ports from parsed configuration blocks
        std::vector<int> ports;
        for (size_t i = 0; i < servers.size(); i++)
        {
            const std::vector<int>& serverPorts = servers[i].getPorts();

            for (size_t j = 0; j < serverPorts.size(); j++)
            {
                ports.push_back(serverPorts[j]);
            }
        }
        // 4. Initialize and pass control over to your integrated server pipeline
        Server server(ports, servers);
        server.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Server Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}