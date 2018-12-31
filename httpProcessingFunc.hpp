#pragma once

#include <string>

namespace multicore {

/**
 * Type of HTTP request.
 */
enum RequestType {
    GET,
    POST,
    DELETE
};

/**
 * Parsed information from an HTTP request.
 *
 * Not all fields are always used. For example if type of request is DELETE, then value is not used.
 */
struct HTTP_Request {
    RequestType type;
    std::string key;
    std::string value;
};

/**
 * Parse a (subset of) HTTP1.1 requests.
 *
 * @param buffer the HTTP request.
 * @param request parsed information.
 * @return 0 if success;
 *         negative values if failed.
 */
int parseHTTP(char *buffer, HTTP_Request &request);

} // namespace multicore
