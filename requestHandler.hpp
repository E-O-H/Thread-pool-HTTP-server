#pragma once

#include <string>
#include <cstdlib>
#include <atomic>

#include "threadSafeKVStore.hpp"
#include "httpProcessingFunc.hpp"

namespace multicore {

/**
 * Handle an HTTP request and build a response.
 * Also maintains three special keys in the storage, "STAT_NUM_INSERT", "STAT_NUM_DELETE" and "STAT_NUM_LOOKUP",
 * which stores the number of inserts, deletes and lookups respectively.
 *
 * @param store the back-end storage.
 * @param request the parsed request information.
 * @return the response.
 */
string handleRequest(ThreadSafeKVStore *store, const HTTP_Request &request);

} // namespace multicore
