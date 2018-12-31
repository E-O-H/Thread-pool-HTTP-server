#pragma once

#include <vector>
#include <pthread.h>
#include <atomic>
#include <chrono>

#include "threadSafeKVStore.hpp"
#include "threadSafeQueue.hpp"

namespace multicore {

struct Task { // Represents a task in the task queue.
    unsigned int socket; // Socket descriptor of the incoming connection.
    std::chrono::time_point<std::chrono::high_resolution_clock> arriveTime; // Arriving time of the task. Used for calculating completing time of the task.
    Task() {}
    Task(unsigned int _socket, std::chrono::time_point<std::chrono::high_resolution_clock> _arriveTime): socket(_socket), arriveTime(_arriveTime) {}
};

class ThreadPoolServer {
  public:

    /**
     * Constructor.
     * @param _portno the port number used by the thread pool server.
     * @param nThreads the number of threads in the thread pool.
     * @param _store pointer to the back-end storage.
     * @param _storagePath path to the storage directory. THIS DIRECTORY WILL BE WIPED CLEAN IF IT ALREADY EXISTS.
     */
    ThreadPoolServer(unsigned short _portno, unsigned int nThreads, ThreadSafeKVStore *_store, string _storagePath);

    /**
     * Destructor.
     */
    ~ThreadPoolServer();

    /**
     * Start the server. Press "ESC" to end the server and print statistics.
     */
    void start();

  private:
    const unsigned short portno;
    const std::string storagePath;
    ThreadSafeQueue<Task> *taskQueue;
    std::vector<pthread_t> *threads;
    ThreadSafeKVStore *store;
    pthread_cond_t task;
    pthread_mutex_t cond_lock, stat_record_lock;

    void *questHandler();
    static void *questHandlerStarter(void *obj);
    void printStats();
};

} // namespace multicore
