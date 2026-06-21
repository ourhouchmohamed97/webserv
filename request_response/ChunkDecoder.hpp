#pragma once

#include <string>
#include <sstream>
#include <utility>

class ChunkDecoder {
private:
    static long hexToDecimal(const std::string& hexStr) {
        std::stringstream ss;
        ss << std::hex << hexStr;
        long val;
        if (!(ss >> val)) return -1;
        return val;
    }

public:
    static std::pair<bool, std::string> decode(const std::string& rawBody) {
        // C++98 syntax for returning std::pair
        if (rawBody.find("\r\n") == std::string::npos) {
            return std::make_pair(true, rawBody);
        }

        std::string decoded = "";
        size_t pos = 0;

        while (pos < rawBody.length()) {
            size_t nextLine = rawBody.find("\r\n", pos);
            if (nextLine == std::string::npos) {
                return std::make_pair(true, decoded.empty() ? rawBody : decoded);
            }

            std::string hexSize = rawBody.substr(pos, nextLine - pos);
            long chunkSize = hexToDecimal(hexSize);
            if (chunkSize < 0) {
                return std::make_pair(true, rawBody);
            }

            if (chunkSize == 0) {
                return std::make_pair(true, decoded);
            }

            size_t chunkDataStart = nextLine + 2;
            if (chunkDataStart + chunkSize > rawBody.length()) {
                return std::make_pair(true, decoded.empty() ? rawBody : decoded);
            }

            decoded.append(rawBody.substr(chunkDataStart, chunkSize));
            pos = chunkDataStart + chunkSize + 2; 
        }

        return std::make_pair(true, decoded);
    }
};