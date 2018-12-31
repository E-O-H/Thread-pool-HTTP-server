#define _GLIBCXX_USE_C99 1

#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>

#include "httpProcessingFunc.hpp"

namespace multicore {

std::string &strToLower(std::string &str) {
    std::transform(str.begin(), str.end(), str.begin(), static_cast<int(*)(int)>(std::tolower));
    return str;
}

int parseHTTP(char *buffer, HTTP_Request &request) {
    std::istringstream strm(buffer);
    std::string token;
    strm >> token;
    if (token == "GET") {
        request.type = GET;
    } else if (token == "POST") {
        request.type = POST;
    } else if (token == "DELETE") {
        request.type = DELETE;
    } else {
        return -1;
    }
    strm >> token;
    if (token.empty() || token[0] != '/') {
        return -2;
    }
    request.key = token.substr(1);
    strm >> token;
    if (token != "HTTP/1.1") {
        return -3;
    }
    if (request.type == POST) {
        size_t length = 0;
        do {
            strm >> token;
            if (strToLower(token) == "content-length:") {
                std::getline(strm, token);
                length = std::stoi(token);
                break;
            }
        } while (!strm.eof());
        do {
            getline(strm, token);
        } while (!token.empty() && token != "\r" && !strm.eof());
        request.value = std::string(length, '\0'); // Note there is actually  (length + 1) '\0's. One will be automatically added.
        strm.read(&request.value[0], length);
    }
    return 0;
}

} // namespace multicore
