#pragma once

#include <string>
#include <sstream>
#include <iostream>

class ChunkDecoder {
public:
    static std::pair<bool, std::string> decode(const std::string& rawBody) {
        // --- GRACEFUL FALLBACK CHECK ---
        // If the body doesn't look like chunked data (e.g., doesn't contain \r\n),
        // or if it fails basic hex line checking, treat the entire body as a flat payload.
        size_t firstLineEnd = rawBody.find("\r\n");
        if (firstLineEnd == std::string::npos) {
            return {true, rawBody}; // It's just a normal flat body string! Fall back safely.
        }

        std::string firstLine = rawBody.substr(0, firstLineEnd);
        size_t extPos = firstLine.find(';');
        if (extPos != std::string::npos) firstLine = firstLine.substr(0, extPos);

        // Check if first line is valid hex. If not, this is a plain text body from curl -d!
        size_t dummySize = 0;
        std::stringstream dummySs;
        dummySs << std::hex << firstLine;
        if (!(dummySs >> dummySize)) {
            return {true, rawBody}; // Not valid hex -> Fall back and process as flat data!
        }

        // --- ACTUAL CHUNKED DECODING LOOP ---
        std::string decoded;
        size_t pos = 0;

        while (pos < rawBody.length()) {
            size_t lineEnd = rawBody.find("\r\n", pos);
            if (lineEnd == std::string::npos) {
                return {true, decoded.empty() ? rawBody : decoded}; // Safe tail fallback
            }

            std::string hexSizeStr = rawBody.substr(pos, lineEnd - pos);
            size_t semi = hexSizeStr.find(';');
            if (semi != std::string::npos) hexSizeStr = hexSizeStr.substr(0, semi);

            size_t chunkSize = 0;
            std::stringstream ss;
            ss << std::hex << hexSizeStr;
            if (!(ss >> chunkSize)) {
                // If mid-parsing fails, return whatever we have aggregated or the raw body
                return {true, decoded.empty() ? rawBody : decoded};
            }

            if (chunkSize == 0) {
                return {true, decoded}; // Terminal chunk reached successfully!
            }

            pos = lineEnd + 2; // Advance past \r\n

            if (pos + chunkSize > rawBody.length()) {
                // Truncated chunk payload, recover whatever we can
                decoded.append(rawBody.substr(pos));
                return {true, decoded};
            }

            decoded.append(rawBody.substr(pos, chunkSize));
            pos += chunkSize;

            if (pos + 2 <= rawBody.length() && rawBody.substr(pos, 2) == "\r\n") {
                pos += 2;
            }
        }

        return {true, decoded.empty() ? rawBody : decoded};
    }
};
