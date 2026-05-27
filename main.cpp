#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>

struct HttpRequest
{
    std::string method;
    std::string path;
    std::string version;

    // We use a map for headers because they are key: value pairs.
    // unordered_map gives us the fast O(1) lookups.
    std::unordered_map<std::string, std::string> headers;

    std::string body; // The optional payload.

    void print() const
    {
        std::cout << "=== HTTP REQUEST ===\n";
        std::cout << "Method: " << method << "\n";
        std::cout << "Path: " << path << "\n";
        std::cout << "Version: " << version << "\n";
        std::cout << "Headers:\n";
        for (const auto &[key, value] : headers)
        {
            std::cout << " [" << key << "] -> " << value << "\n";
        }

        std::cout << "Body: " << body << "\n";
        std::cout << "=========================\n";
    }
};

class RequestParser
{
private:
    // Helper function to trim trailing '\r' and leading/trailing spaces
    static std::string trim(std::string str)
    {
        if (!str.empty() && str.back() == '\r')
        {
            str.pop_back();
        }
        // Basic space trimming
        size_t first = str.find_first_not_of(" ");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" ");
        return str.substr(first, (last - first + 1));
    }

public:
    static HttpRequest parse(const std::string &rawRequest)
    {
        HttpRequest req;
        std::stringstream requestStream(rawRequest);
        std::string line;
        // Parse Request Line
        if (std::getline(requestStream, line))
        {
            line = trim(line);
            std::stringstream lineStream(line);
            lineStream >> req.method >> req.path >> req.version;
        }
        // Parse Headers up to Blank Line
        while (std::getline(requestStream, line))
        {
            line = trim(line);

            if (line.empty())
                break;

            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos)
            {
                std::string key = trim(line.substr(0, colonPos));
                std::string value = trim(line.substr(colonPos + 1));
                req.headers[key] = value;
            }
        }

        // POST Validation: Check Content-Length
        if (req.method == "POST")
        {
            auto it = req.headers.find("Content-Length");
            if (it == req.headers.end())
            {
                throw std::runtime_error("404 Bad Request: Missing Content-Length");
            }

            int contentLength = std::stoi(it->second);

            if (contentLength > 0)
            {
                std::string remainingContent;
                std::ostringstream remainder;
                remainder << requestStream.rdbuf();
                remainingContent = remainder.str();

                if (remainingContent.length() >= static_cast<size_t>(contentLength))
                {
                    req.body = remainingContent.substr(0, contentLength);
                }
                else
                {
                    throw std::runtime_error("404 Bad Request: Body size less than Content-Length.");
                }
            }
        }
        return req;
    }
};

class HttpResponse
{
private:
    std::string version = "HTTP/1.1";
    int statusCode;
    std::string statusMessage;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

public:
    HttpResponse(int code, std::string message) : statusCode(code), statusMessage(message) {}

    void setHeader(const std::string &key, const std::string &value)
    {
        headers[key] = value;
    }

    // Set Content-Length automatically from body size
    void setBody(const std::string &b)
    {
        body = b;
        setHeader("Content-Length", std::to_string(body.length()));
    }
    // Serialize response to raw bytes: status + headers + \r\n\r\n + body

    std::string toString() const
    {
        std::stringstream ss;
        ss << version << " " << statusCode << " " << statusMessage << "\r\n";

        for (const auto &[key, value] : headers)
        {
            ss << key << ": " << value << "\r\n";
        }

        ss << "\r\n"
           << body;
        return ss.str();
    }
};

class MimeTypeHelper
{
private:
    static std::unordered_map<std::string, std::string> mimeMap;

public:
    static std::string getMimeType(const std::string &extension)
    {
        auto it = mimeMap.find(extension);
        if (it != mimeMap.end())
        {
            return it->second;
        }
        return "application/octet-stream"; // Default for unknown/binary files
    }
};

std::unordered_map<std::string, std::string> MimeTypeHelper::mimeMap = {
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".png", "image/png"},
    {".jpeg", "image/jpeg"},
    {".ico", "image/x-icon"},
    {".txt", "image/plain"},
    {".pdf", "applicaiton/pdf"},
};


class ErrorPageFactory {
private:
    // A reusable master template to avoid duplicating CSS styles
    static std::string buildTemplate(const std::string& code, const std::string& title, const std::string& description) {
        return R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Error )" + code + R"(: )" + title + R"(</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background-color: #121214;
            color: #e1e1e6;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
            margin: 0;
        }
        .container {
            text-align: center;
            max-width: 450px;
            padding: 20px;
        }
        h1 {
            font-size: 6rem;
            margin: 0;
            color: #ff5555;
            font-weight: 800;
            letter-spacing: -2px;
        }
        h2 {
            font-size: 1.5rem;
            margin: 10px 0 20px 0;
            color: #ff79c6;
        }
        p {
            color: #8b92a5;
            line-height: 1.6;
            margin-bottom: 30px;
        }
        .button {
            display: inline-block;
            background-color: #44475a;
            color: #f8f8f2;
            text-decoration: none;
            padding: 10px 20px;
            border-radius: 6px;
            font-size: 0.9rem;
            transition: background 0.2s;
        }
        .button:hover {
            background-color: #6272a4;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>)" + code + R"(</h1>
        <h2>)" + title + R"(</h2>
        <p>)" + description + R"(</p>
        <a href="/" class="button">Back to Home</a>
    </div>
</body>
</html>)";
    }
public:
    static std::string getErrorPage(int statusCode) {
        switch (statusCode) {
            case 400:
                return buildTemplate("400", "Bad Request", "The server could not understand the request due to malformed syntax.");
            case 403:
                return buildTemplate("403", "Forbidden", "You don't have permission to access this resource.");
            case 404:
                return buildTemplate("404", "Not Found", "The requested URL was not found on this server.");
            case 405:
                return buildTemplate("405", "Method Not Allowed", "The HTTP method used is not supported for this URL.");
            case 500:
                return buildTemplate("500", "Internal Server Error", "The server encountered an error and could not complete your request.");
            default:
                return buildTemplate(std::to_string(statusCode), "Error", "An unexpected error occurred.");
        }
    }
};


class PathResolver {
    public:
        // Decodes 20% to space, etc
        static std::string urlDecode(std::string str) {
            std::string ret;
            for (size_t i = 0; i < str.length(); ++i) {
                if (str[i] == '%' && i + 2 < str.length()) {
                    int value = std::stoi(str.substr(i + 1, 2), NULL, 16);
                    ret += static_cast<char>(value);
                    i += 2;
                }
                else {
                    ret += str[i];
                }
            }
            return ret;
        }

        // Resolves . and .. compenents
        static std::string normalize(std::string path) {
            std::vector<std::string> parts;
            std::stringstream ss(path);
            std::string segment;
            while (std::getline(ss, segment, '/')) {
                if (segment == "" || segment == ".")
                    continue;
                if (segment == "..") {
                    if (!parts.empty())
                        parts.pop_back();
                }
                else
                    parts.push_back(segment);
            }
            std::string result = "/";
            for (const auto& p : parts)
                result += p + "/";
            if (result.length() > 1)
                result.pop_back();
            return result;
        }
};


class AutoIndex {
    public:
        static std::string generate(const std::string& path, const std::string& dirPath) {
            std::string html = "<html><body><h1>Index of " + dirPath + "</h1><hr><ul>";
            DIR* dir = opendir(path.c_str());
            if (dir == NULL)
                return "";

            struct dirent* entry;
            while((entry = readdir(dir)) != NULL) {
                std::string name = entry->d_name;
                if (name == ".")
                    continue; // Skip current dir link

                    // Generate a simple link for eack file/folder
                    html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
            }

            closedir(dir);
            html += "</ul><hr></body></html>";
            return html;
        }
};

class StaticFileServer {
    public:
        static HttpResponse serveFile(const std::string& targetPath) {
            // Hardcoded root directory for our sanity check purpose
            const std::string rootDir = "./www";
            
            // 1. Normalize and Decode
            std::string decodedPath = PathResolver::urlDecode(targetPath);
            std::string normalized = PathResolver::normalize(decodedPath);

            // 2. Build absolute path and check for traversal
            std::string fullPath = rootDir + normalized;
            // Check if path exist
            struct stat pathStat;
            if (stat(fullPath.c_str(), &pathStat) != 0)
                return HttpResponse(404, "Not Found");
            // If it's a directory, look for index.html
            if(pathStat.st_mode & S_IFDIR) {
                std::string indexPath = fullPath + "/index.html";

                // If index.html exists. serve it
                if (access(indexPath.c_str(), F_OK) == 0) {
                    fullPath = indexPath;
                }
                // Otherwise, if autoindex is ON, generate a list
                else if (/* config file: autoindex = true */0 == 0) {
                    HttpResponse res(200, "OK");
                    res.setHeader("Content-Type", "text/html");
                    res.setBody(AutoIndex::generate(fullPath, normalized));
                    return res;
                }
                else
                    return HttpResponse(403, "Forbidden");
            }
            // Check read permissions
            if (access(fullPath.c_str(), R_OK) == -1)
                return HttpResponse(403, "Forbidden");

            // 4. If all checks pass, serve the file
            return serveFileContent(fullPath, normalized); 
        }
    
    private:
        static HttpResponse serveFileContent(const std::string& fullPath, const std::string& normalized) {
            std::ifstream file(fullPath, std::ios::in | std::ios::binary);

            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            HttpResponse res(200, "OK");
            // Use normalized Path to determine the MIME Type
            size_t dotPos = normalized.find_last_of('.');
            std::string extension = (dotPos != std::string::npos) ? normalized.substr(dotPos) : "";
            res.setHeader("Content-Type", MimeTypeHelper::getMimeType(extension));

            res.setBody(buffer.str());
            return res;
        }
};


int main()
{
    // // SCENARIO A: Valid POST request with Content-Length
    // std::string validPost =
    //     "POST /api/users HTTP/1.1\r\n"
    //     "Host: localhost\r\n"
    //     "Content-Length: 15\r\n"
    //     "\r\n"
    //     "id=123&name=bob"; // 15 characters long

    // try {
    //     std::cout << "Testing Valid POST:\n";
    //     HttpRequest reqA = RequestParser::parse(validPost);
    //     reqA.print();
    // } catch (const std::exception& e) {
    //     std::cout << "Error: " << e.what() << "\n";
    // }

    // std::cout << "\n\n";

    // // SCENARIO B: Invalid POST request (Missing Content-Length)
    // std::string invalidPost =
    //     "POST /api/users HTTP/1.1\r\n"
    //     "Host: localhost\r\n"
    //     "\r\n"
    //     "id=123&name=bob";

    // try {
    //     std::cout << "Testing Invalid POST:\n";
    //     HttpRequest reqB = RequestParser::parse(invalidPost);
    //     reqB.print();
    // } catch (const std::exception& e) {
    //     std::cout << "Rejected! -> " << e.what() << "\n";
    // }

    // std::cout << "\n\n";
    // std::cout << "Testing Response:\n";

    // HttpResponse res(200, "OK");
    // res.setHeader("Content-Type", "applocation/json");
    // res.setBody("{\"message\": \"Hello World\"}");

    // std::string rawResponse = res.toString();

    // std::cout << "--- Sending this to Client ---\n" << rawResponse << "\n";

    // === Mime Type ===

    // // 1. The raw input string
    // std::string rawReq =
    //     "GET /styles.css HTTP/1.1\r\n"
    //     "Host: localhost\r\n"
    //     "\r\n";
    // // 2. Parse the string into our object
    // HttpRequest req = RequestParser::parse(rawReq);

    // // 3. Create our response object
    // HttpResponse res(200, "OK");

    // // 4. Extract path from the REQUEST object
    // std::string path = req.path;

    // // 5. Logic to find the extension
    // size_t dotPos = path.find_last_of('.');
    // std::string extension = (dotPos != std::string::npos) ? path.substr(dotPos) : "";

    // // 6. Set Content-Type header from file extension in the RESPONSE object
    // std::string contentType = MimeTypeHelper::getMimeType(extension);
    // res.setHeader("Content-Type", contentType);

    // // 7. Print to verify
    // std::cout << "Target File Path: " << path << "\n";
    // std::cout << "Detected Extension: " << extension << "\n";
    // std::cout << "Assigned Content-Type Header: " << contentType << "\n";

    // int errorCode = 404;
    
    // // 1. Generate the HTML string from your class factory
    // std::string errorHtml = ErrorPageFactory::getErrorPage(errorCode);
    
    // // 2. Open a file stream to write on your computer disk
    // std::ofstream outFile("error404.html");
    
    // if (outFile.is_open()) {
    //     // Write the raw HTML string directly into the file
    //     outFile << errorHtml;
    //     outFile.close();
        
    //     std::cout << "--------------------------------------------------\n";
    //     std::cout << " SUCCESS: 'error404.html' has been written to disk!\n";
    //     std::cout << "--------------------------------------------------\n";
    //     std::cout << "Next steps:\n";
    //     std::cout << "1. Open your terminal directory folder.\n";
    //     std::cout << "2. Double-click 'error404.html' to open it in a browser.\n";
    // } else {
    //     std::cerr << "Error: Could not create file on disk.\n";
    // }


    HttpRequest req;
    req.method = "GET";
    req.path = "/index.js";

    std::cout << "Client requested file: " << req.path << "\n";

    HttpResponse response = StaticFileServer::serveFile(req.path);

    std::string rawOutput = response.toString();
    std::cout << "\n--- Raw Output Stream to Network Wire ---\n";
    std::cout << rawOutput << "\n";
    std::cout << "------------------------------------------\n";
    return 0;
}
