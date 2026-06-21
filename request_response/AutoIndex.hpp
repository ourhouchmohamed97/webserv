#pragma once

#include <string>
#include <dirent.h>

class AutoIndex {
public:
    static std::string generate(const std::string& path, const std::string& dirPath) {
        std::string html = "<html><body><h1>Index of " + dirPath + "</h1><hr><ul>";
        DIR* dir = opendir(path.c_str());
        if (dir == NULL) return "";

        struct dirent* entry;
        while((entry = readdir(dir)) != NULL) {
            std::string name = entry->d_name;
            if (name == ".") continue;
            html += "<li><a href=\"" + name + "\">" + name + "</a></li>";
        }
        closedir(dir);
        html += "</ul><hr></body></html>";
        return html;
    }
};
