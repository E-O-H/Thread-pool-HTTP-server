#include "requestHandler.hpp"

namespace multicore {

extern std::atomic_ulong stat_num_lookup;
extern std::atomic_ulong stat_num_insert;
extern std::atomic_ulong stat_num_delete;

string handleRequest(ThreadSafeKVStore *store, const HTTP_Request &request) {
    int res;
    string val;
    string str;
    switch (request.type) {
      case GET:
        res = store->lookup(request.key, val);
        ++stat_num_lookup;
        break;
      case POST:
        res = store->insert(request.key, request.value);
        ++stat_num_insert;
        break;
      case DELETE:
        res = store->remove(request.key);
        ++stat_num_delete;
        break;
      default:
        exit(-1);
    }
    if (res) {
        str = "HTTP/1.1 404 Not found\r\nContent-length: 0\r\n\r\n";
    } else {
        str = "HTTP/1.1 200 OK\r\nContent-length: ";
        if (request.type == GET) {
            str += std::to_string(val.length());
            str += "\r\n\r\n";
            str += val;
            str += "\r\n";
        } else {
            str += "0\r\n\r\n";
        }
    }
    return str;
}

} // namespace multicore
