#include <cstdlib>
#include <cctype>
#include <cstdio>
#include <unistd.h>
#include <algorithm>
#include <numeric>

#include "threadSafeKVStore.hpp"
#include "threadPoolServer.hpp"

#define STRINGIFY_DIRECT(X)  #X
#define STRINGIFY(X)         STRINGIFY_DIRECT(X)

#define DEFAULT_NUM_THREADS  1                   // Default number of threads if no argument is given.
#define DEFAULT_PORT_NO              10801               // Port Number used by the program.
#define DEFAULT_STORAGE_PATH         "./storage"         // path of disk storage. THIS DIRECTORY WILL BE WIPED CLEAN IF ALREADY EXISTS.
#define DEFAULT_CACHE_SIZE           128                 // Size of in memory cache. Change this value to 0 to disable memory cache.

namespace multicore {

std::atomic_ulong stat_num_lookup;
std::atomic_ulong stat_num_insert;
std::atomic_ulong stat_num_delete;
std::atomic_bool isRunning;
std::vector<float> requestTimes;

// Parses the arguments for the program.
int argParser(int argc, char **argv) {
    char *nvalue = NULL;
    int c;
    opterr = 0;
    while ((c = getopt (argc, argv, "n:")) != -1)
		switch (c) {
          case 'n':
            nvalue = optarg;
            break;
          case '?':
            if (optopt == 'n')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
            return 1;
          default:
            abort();
		}
    if (nvalue == NULL) {
        printf("Option -n not specified, using default value " STRINGIFY(DEFAULT_NUM_THREADS) ".\n");
        return DEFAULT_NUM_THREADS;
    } else {
        return atoi(nvalue);
    }
}

void printStats() {
    printf("*******************************CURRENT STATS********************************\n");
    printf("Total number of inserts = %lu\nTotal number of deletes = %lu\nTotal number of lookups = %lu\n",
            stat_num_insert.load(),
            stat_num_delete.load(),
            stat_num_lookup.load());
    std::sort(requestTimes.begin(), requestTimes.end());
    float min = requestTimes.empty() ? 0 : *requestTimes.begin();
    float max = requestTimes.empty() ? 0 : *(requestTimes.end() - 1);
    float mean = requestTimes.empty() ? 0 : std::accumulate(requestTimes.begin(), requestTimes.end(), 0.0) / requestTimes.size();
    float median = requestTimes.empty() ? 0 : (requestTimes.size() % 2 ?
                                               requestTimes[requestTimes.size() / 2] :
                                               (requestTimes[requestTimes.size() / 2 - 1] +
                                                requestTimes[requestTimes.size() / 2]) / 2.0);
    printf("Request time (ms): min = %f, avg = %f, max = %f, median = %f\n",
            min, mean, max, median);
    printf("****************************************************************************\n");
}

void clearStats() {
    requestTimes.clear();
    stat_num_insert = 0;
    stat_num_delete = 0;
    stat_num_lookup = 0;
    printf(">>>> Stats cleared. (Note the key-value storage is not reset, only the statistics.)\n");
}

void *startThreadPoolServer(void *nThread) {
    multicore::ThreadSafeKVStore *store = new multicore::ThreadSafeKVStore(DEFAULT_STORAGE_PATH, DEFAULT_CACHE_SIZE); // Create back-end storage.
    multicore::ThreadPoolServer *server = new multicore::ThreadPoolServer(DEFAULT_PORT_NO, * (int*) nThread, store, DEFAULT_STORAGE_PATH); // Create thread pool.
    server->start(); // Start listening to connections.
}

} // multicore

// Program entry.
int main(int argc, char **argv) {
    int nThread = multicore::argParser(argc, argv);
    multicore::isRunning = true;
    pthread_t tid;
    // Create thread-pool-server thread.
    if (pthread_create(&tid, nullptr, multicore::startThreadPoolServer, (void *) &nThread)) {
            fprintf(stderr, "thread-pool-server thread creation failed. Terminating.\n");
            exit(-1);
        }
    // Waiting for user input.
    while (multicore::isRunning.load()) {
        printf(">>>> Server running... \nEnter 's' to print statistics,\n'r' to reset the statistics recording\nOr 'q' to terminate the server (all key-value storage will be lost).\n:");
        char keyPressed = getchar();
        if (keyPressed == 's') {
            multicore::printStats();
        } else if (keyPressed == 'r') {
            multicore::clearStats();
        } else if (keyPressed == 'q') {
            multicore::isRunning = false;
        }
        while ((keyPressed = getchar()) != '\n' && keyPressed != EOF) {}
    }
    return 0;
}
