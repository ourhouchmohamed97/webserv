#include <iostream>
#include <string>
#include <unordered_map>
#include <sstream>
#include <stdexcept>
#include <algorithm>

struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;

    // We use a map for headers because they are key: value pairs.
    // unordered_map gives us the fast O(1) lookups.
    std::unordered_map<std::string, std::string> headers;

    std::string body; // The optional payload.

    void print() const {
        std::cout << "=== HTTP REQUEST ===\n";
        std::cout << "Method: " << method << "\n";
        std::cout << "Path: " << path << "\n";
        std::cout << "Version: " << version << "\n";
        std::cout << "Headers:\n";
        for (const auto& [key, value] : headers) {
            std::cout << " [" << key << "] -> " << value << "\n";
        }

        std::cout << "Body: " << body << "\n";
        std::cout << "=========================\n";
    }
};

class RequestParser {
    private:
        // Helper function to trim trailing '\r' and leading/trailing spaces
        static std::string trim(std::string str) {
            if (!str.empty() && str.back() == '\r') {
                str.pop_back();
            }
            // Basic space trimming
            size_t first = str.find_first_not_of(" ");
            if (first == std::string::npos) return "";
            size_t last = str.find_last_not_of(" ");
            return str.substr(first, (last - first + 1));
        }
    public:
        static HttpRequest parse(const std::string& rawRequest) {
            HttpRequest req;
            std::stringstream requestStream(rawRequest);
            std::string line;
            // Parse Request Line
            if (std::getline(requestStream, line)) {
                line = trim(line);
                std::stringstream lineStream(line);
                lineStream >> req.method >> req.path >> req.version;
            }
            // Parse Headers up to Blank Line
            while(std::getline(requestStream, line)) {
                line = trim(line);

                if (line.empty())
                    break;
                
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string key = trim(line.substr(0, colonPos));
                    std::string value = trim(line.substr(colonPos + 1));
                    req.headers[key] = value;
                }
            }

            // POST Validation: Check Content-Length
            if (req.method == "POST") {
                auto it = req.headers.find("Content-Length");
                if (it == req.headers.end()) {
                    throw std::runtime_error("404 Bad Request: Missing Content-Length");
                }

                int contentLength = std::stoi(it->second);

                if (contentLength > 0) {
                    std::string remainingContent;
                    std::ostringstream remainder;
                    remainder << requestStream.rdbuf();
                    remainingContent = remainder.str();
                    
                    if (remainingContent.length() >= static_cast<size_t>(contentLength)) {
                        req.body = remainingContent.substr(0, contentLength);
                    } else {
                        throw std::runtime_error("404 Bad Request: Body size less than Content-Length.");
                    }
                }
            }
            return req;
        } 
};

class HttpResponse {
    private:
        std::string version = "HTTP/1.1";
        int statusCode;
        std::string statusMessage;
        std::unordered_map<std::string, std::string> headers;
        std::string body;

    public:
        HttpResponse (int code, std::string message) : statusCode(code), statusMessage(message) {}

        void setHeader(const std::string& key, const std::string& value) {
            headers[key] = value;
        }

        // Set Content-Length automatically from body size
        void setBody(const std::string& b) {
            body = b;
            setHeader("Content-Length", std::to_string(body.length()));
        }
        // Serialize response to raw bytes: status + headers + \r\n\r\n + body

        std::string toString() const {
            std::stringstream ss;
            ss << version << " " << statusCode << " " << statusMessage << "\r\n";

            for (const auto& [key, value] : headers) {
                ss << key << ": " << value << "\r\n";
            }

            ss << "\r\n" << body;
            return ss.str();
        }
};

class MimeTypeHelper {
    private:
        static std::unordered_map<std::string, std::string> mimeMap;

    public:
        static std::string getMimeType(const std::string& extension) {
            auto it = mimeMap.find(extension);
            if (it != mimeMap.end()) {
                return it->second;
            }
            return "application/octet-stream"; // Default for unknown/binary files
        }
};

// Initialize the map
std::unordered_map<std::string, std::string> MimeTypeHelper::mimeMap = {
    {".html", "text/html"},
    {".css",  "text/css"},
    {".js",   "application/javascript"},
    {".png",  "image/png"},
    {".jpg",  "image/jpeg"},
    {".ico",  "image/x-icon"},
    {".txt",  "text/plain"},
    {".pdf",  "application/pdf"}
};


int main() {
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

    // 1. The raw input string
    std::string rawReq = 
        "GET /styles.css HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "\r\n";
    // 2. Parse the string into our object
    HttpRequest req = RequestParser::parse(rawReq);

    // 3. Create our response object 
    HttpResponse res(200, "OK");

    // 4. Extract path from the REQUEST object
    std::string path = req.path;

    // 5. Logic to find the extension
    size_t dotPos = path.find_last_of('.');
    std::string extension = (dotPos != std::string::npos) ? path.substr(dotPos) : "";

    // 6. Set the header in the RESPONSE object
    std::string contentType = MimeTypeHelper::getMimeType(extension);
    res.setHeader("Content-Type", contentType);

    // 7. Print to verify
    std::cout << "Target File Path: " << path << "\n";
    std::cout << "Detected Extension: " << extension << "\n";
    std::cout << "Assigned Content-Type Header: " << contentType << "\n";
    
    return 0;
}
