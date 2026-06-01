#pragma once

#include <string>
#include <sstream>
#include <iostream>

class ChunkDecoder {
    public:
        static std::pair<bool, std::string> decode(const std::string& rawBody) {
            std::string decoded;
            size_t pos = 0;

            while (pos < rawBody.size()) {
                // Find the end of the hex size line (\r\n)
                size_t lineEnd = rawBody.find("\r\n", pos);
                if (lineEnd == std::string::npos) {
                    // Incomplete stream chunk header
                    return {false, ""};
                }

                std::string hexSizeStr = rawBody.substr(pos, lineEnd - pos);

                // Strip out any chunk extensions if present (delimited by ';')
                size_t extPos = hexSizeStr.find(';');
                if (extPos != std::string::npos) {
                    hexSizeStr = hexSizeStr.substr(0, extPos);
                }

                // Parse the hexadecimal string size into an integer
                size_t chunkSize = 0;
                std::stringstream ss;
                ss << std::hex << hexSizeStr;
                if (!(ss >> chunkSize)) {
                    return {false, ""}; // Malformed hex string
                }

                // Check for the final terminating chunk marker (0\r\n\r\n)
                if (chunkSize == 0) {
                    return {true, decoded};
                }

                // Advance position past the chunk size line "\r\n"
                pos = lineEnd + 2;

                // Check if the entire chunk payload exists in the current buffer
                if (pos + chunkSize > rawBody.length()) {
                    return {false, ""}; // Payload trucated. need more socket reading
                }

                // Extract the chunk content data append it to our accumulator
                decoded.append(rawBody.substr(pos, chunkSize));
                pos += chunkSize;


                // Verify and skip past the mandatory trailing "\r\n" folowing the payload
                if (pos + 2 > rawBody.length() || rawBody.substr(pos, 2) != "\r\n") {
                    return {false, ""}; // Missing or malformed chunk terminator
                }
                pos += 2; // Move past the chunk terminator for the next iteration
            }
            return {false, ""}; // Hit end of buffer without recieving the terminating 0 chunk
        }
};