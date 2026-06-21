#pragma once

#include <string>
#include <sstream>

class ErrorPageFactory {
private:
    static std::string buildTemplate(const std::string& code, const std::string& title, const std::string& description) {
        std::string html;
        html += "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n";
        html += "<meta charset=\"UTF-8\">\n<title>Error " + code + ": " + title + "</title>\n";
        html += "<style>\n";
        html += "body { font-family: -apple-system, sans-serif; background-color: #121214; color: #e1e1e6; display: flex; align-items: center; justify-content: center; height: 100vh; margin: 0; }\n";
        html += ".container { text-align: center; max-width: 450px; padding: 20px; }\n";
        html += "h1 { font-size: 6rem; margin: 0; color: #ff5555; letter-spacing: -2px; }\n";
        html += "h2 { font-size: 1.5rem; margin: 10px 0 20px 0; color: #ff79c6; }\n";
        html += "p { color: #8b92a5; line-height: 1.6; margin-bottom: 30px; }\n";
        html += ".button { background-color: #44475a; color: #f8f8f2; text-decoration: none; padding: 10px 20px; border-radius: 6px; font-size: 0.9rem; transition: background 0.2s; }\n";
        html += ".button:hover { background-color: #6272a4; }\n";
        html += "</style>\n</head>\n<body>\n<div class=\"container\">\n";
        html += "<h1>" + code + "</h1>\n";
        html += "<h2>" + title + "</h2>\n";
        html += "<p>" + description + "</p>\n";
        html += "<a href=\"/\" class=\"button\">Return Home</a>\n";
        html += "</div>\n</body>\n</html>";
        return html;
    }

public:
    static std::string getErrorPage(int statusCode) {
        std::stringstream ss;
        ss << statusCode;
        std::string codeStr = ss.str();

        if (statusCode == 400) return buildTemplate(codeStr, "Bad Request", "The server could not understand the request token format.");
        if (statusCode == 403) return buildTemplate(codeStr, "Forbidden", "Access verification denied for the requested resource root.");
        if (statusCode == 404) return buildTemplate(codeStr, "Not Found", "The requested target asset path could not be located on this server.");
        if (statusCode == 405) return buildTemplate(codeStr, "Method Not Allowed", "The requested dynamic method action is barred for this route endpoint.");
        if (statusCode == 411) return buildTemplate(codeStr, "Length Required", "A valid Content-Length verification metric was missing from the header array.");
        if (statusCode == 413) return buildTemplate(codeStr, "Payload Too Large", "The requested upload body entity exceeds our configured root safety caps.");
        if (statusCode == 500) return buildTemplate(codeStr, "Internal Server Error", "An unhandled transaction anomaly crashed the server context runtime.");
        
        return buildTemplate(codeStr, "Error", "An unexpected network error occurred.");
    }
};