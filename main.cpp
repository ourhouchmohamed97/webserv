#include "request_response/Config.hpp"
#include "request_response/RequestParser.hpp"
#include "request_response/StaticFileServer.hpp"
#include <iostream>
#include <map>
#include <vector>
#include <string>

// Global config definitions required by the headers
ServerConfig globalConfig;
std::map<std::string, RedirectRule> globalRedirects;

void runTestCase(const std::string& testName, const std::string& rawRequest) {
    std::cout << "\n========================================\n";
    std::cout << " RUNNING TEST: " << testName << "\n";
    std::cout << "========================================\n";

    try {
        // 1. Test your Parser
        HttpRequest req = RequestParser::parse(rawRequest);
        std::cout << "[PARSER SUCCESS]\n";
        req.print();

        // 2. Test your Routing / Static File Engine
        HttpResponse res = StaticFileServer::serveFile(req, globalConfig, globalRedirects);
        std::cout << "[SERVER RESPONSE SUCCESS]\n";
        std::cout << res.toString() << "\n";

    } catch (const std::exception& e) {
        std::cout << "[TEST CRASHED] Exception caught: " << e.what() << "\n";
    }
}

int main() {
    // ---- SETUP C++98 CONFIGURATION FOR TESTING ----
    RouteConfig rootRoute;
    rootRoute.root = "./www";
    rootRoute.indexFile = "index.html";
    rootRoute.autoindex = true;
    rootRoute.clientMaxBodySize = 1000000;
    rootRoute.allowedMethods.push_back("GET");

    RouteConfig imageRoute;
    imageRoute.root = "./www/global_images";
    imageRoute.autoindex = true;
    imageRoute.clientMaxBodySize = 50000000;
    imageRoute.allowedMethods.push_back("GET");
    imageRoute.allowedMethods.push_back("POST");
    imageRoute.allowedMethods.push_back("DELETE");

    globalConfig.addRoute("/", rootRoute);
    globalConfig.addRoute("/images", imageRoute);
    globalRedirects["/old-home"] = RedirectRule(301, "/index.html");

    // ---- TEST CASE 1: Standard GET Request ----
    std::string getReq = 
        "GET / index.html HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: curl/7.68.0\r\n"
        "Accept: */*\r\n"
        "\r\n";
    runTestCase("Basic GET Request", getReq);

    // ---- TEST CASE 2: Route Matcher & AutoIndex ----
    std::string autoIndexReq = 
        "GET /images HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "\r\n";
    runTestCase("AutoIndex Directory Listing Fetch", autoIndexReq);

    // ---- TEST CASE 3: Multipart POST File Upload ----
    // This explicitly tests your custom C++98 stringstream content-length 
    // parser and your updated MultipartParser data boundary parsing loops.
    std::string boundary = "----WebKitFormBoundaryABC123";
    std::string multipartBody = 
        "--" + boundary + "\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"test_upload.txt\"\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "Hello, this is a C++98 isolated string parsing test text file!\r\n"
        "--" + boundary + "--\r\n";

    std::stringstream ssLen;
    ssLen << multipartBody.length();

    std::string postReq = 
        "POST /images HTTP/1.1\r\n"
        "Host: localhost:8080\r\n"
        "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n"
        "Content-Length: " + ssLen.str() + "\r\n\r\n" +
        multipartBody;

    runTestCase("Multipart POST File Upload", postReq);

    return 0;
}