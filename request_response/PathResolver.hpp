#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

class PathResolver {
public:
    static std::string urlDecode(std::string str) {
        std::string ret;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value = 0;
                // Safe C++98 parsing without std::stoi
                std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value);
                ret += static_cast<char>(value);
                i += 2;
            } else {
                ret += str[i];
            }
        }
        return ret;
    }

    static std::string normalize(std::string path) {
        std::vector<std::string> parts;
        std::stringstream ss(path);
        std::string segment;
        while (std::getline(ss, segment, '/')) {
            if (segment == "" || segment == ".") continue;
            if (segment == "..") {
                if (!parts.empty()) parts.pop_back();
            } else {
                parts.push_back(segment);
            }
        }
        std::string result = "/";
        for (size_t i = 0; i < parts.size(); ++i) {
            result += parts[i] + "/";
        }
        if (result.length() > 1) {
            result.resize(result.length() - 1);
        }
        return result;
    }
};