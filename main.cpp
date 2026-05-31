#include "Config.hpp"
#include "HttpServer.hpp"
#include <iostream>

int main() {
    ServerConfig config;

    // Route 1: The general fallback root route (Only allows GET)
    RouteConfig rootRoute;
    rootRoute.root = "./www/main_site";
    rootRoute.autoindex = false;
    rootRoute.indexFile = "index.html";
    rootRoute.allowedMethods = {"GET"}; // <-- ALLOWED METHODS LISTED
    rootRoute.clientMaxBodySize = 1048576; // 1 MB fallback limit
    config.addRoute("/", rootRoute);

    // Route 2: The images directory asset route (Allows GET and POST)
    RouteConfig imageRoute;
    imageRoute.root = "./www/global_images";
    imageRoute.autoindex = true; 
    imageRoute.indexFile = "index.html";
    imageRoute.allowedMethods = {"GET", "POST"}; // <-- ALLOWED METHODS LISTED
    imageRoute.clientMaxBodySize = 100; // Strict upload limit: 100 Bytes!
    config.addRoute("/images", imageRoute);

    std::unordered_map<std::string, RedirectRule> redirects;
    redirects["/old-home"] = {301, "/index.html"};

    const int PORT = 8080;
    HttpServer server(PORT, config, redirects);
    server.init();
    server.start();

    return 0;
}