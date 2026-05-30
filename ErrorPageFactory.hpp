#pragma once

#include <string>
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
