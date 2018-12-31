#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <string>
#include <chrono>

#include "threadPoolServer.hpp"
#include "httpProcessingFunc.hpp"
#include "requestHandler.hpp"
#include "fileSystemIO.hpp"

#define BUFFER_LENGTH 4096 // Max length of a HTTP request

namespace multicore {

typedef unsigned int FileDescriptor;
extern std::atomic_bool isRunning;
extern std::atomic_ulong stat_num_lookup;
extern std::atomic_ulong stat_num_insert;
extern std::atomic_ulong stat_num_delete;
extern std::vector<float> requestTimes;

ThreadPoolServer::ThreadPoolServer(unsigned short _portno, unsigned int nThreads, ThreadSafeKVStore *_store, string _storagePath):
                                   portno(_portno), store(_store), storagePath(_storagePath) {
    pthread_cond_init(&task, nullptr);
    pthread_mutex_init(&cond_lock, nullptr);
    pthread_mutex_init(&stat_record_lock, nullptr);
    taskQueue = new ThreadSafeQueue<Task>;
    threads = new std::vector<pthread_t>(nThreads, 0);
    // Initialize thread pool.
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    for (unsigned int i = 0; i < nThreads; ++i) {
        if (pthread_create(&threads->at(i), &attr, questHandlerStarter, (void *)this)) {
            fprintf(stderr, "pthread_create failed. Terminating.\n");
            exit(-1);
        }
    }
    pthread_attr_destroy(&attr);
    // Initialize stats
    stat_num_lookup = 0;
    stat_num_insert = 0;
    stat_num_delete = 0;
}

ThreadPoolServer::~ThreadPoolServer() {
    // Join all spawned threads in the thread-pool.
    void *status;
    for (pthread_t tid : *threads) {
        if (pthread_join(tid, &status)) {
            fprintf(stderr, "ERROR: Problem with joining threads in pool. There may be in-memory cache not written back to disk. \n");
            exit(-1);
        }
    }
    pthread_cond_destroy(&task);
    pthread_mutex_destroy(&cond_lock);
    pthread_mutex_destroy(&stat_record_lock);
    delete taskQueue;
    delete threads;
}

void ThreadPoolServer::start() {
    // Initialize disk storage
    if (initDir(storagePath)) {
        fprintf(stderr, "Disk storage initialization failed. Terminating.\n");
        exit(-1);
    }

    // Create an Internet socket
    FileDescriptor sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Creating socked failed. Terminating.\n");
        exit(-1);
    }
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        fprintf(stderr, "Binding failed. Terminating.\n");
        exit(-1);
    }
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    // Listen to incoming connections
    while (isRunning.load()) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            fprintf(stderr, "Connection failed. Skipping current connection.\n");
            continue;
        }
        pthread_mutex_lock(&cond_lock);
        taskQueue->enqueue(Task(newsockfd, std::chrono::high_resolution_clock::now())); // Register the arriving connection as a new task.
        pthread_cond_signal(&task); // Signal the thread pool a new task has arrived.
        pthread_mutex_unlock(&cond_lock);
    }
    close(sockfd);
}

// The routine for each thread in the thread pool to run.
void *ThreadPoolServer::questHandler() {
    int n;
    char buffer[BUFFER_LENGTH];
    unsigned int sock;
    HTTP_Request request;
    std::string response;
    while (isRunning.load()) {
        pthread_mutex_lock(&cond_lock);
        while (taskQueue->empty()) {
            pthread_cond_wait(&task, &cond_lock);
        }
        pthread_mutex_unlock(&cond_lock);
        Task t = taskQueue->dequeue();
        std::chrono::time_point<std::chrono::high_resolution_clock> arriveTime = t.arriveTime;
        sock = t.socket;
        while(true) {
            memset(buffer, 0, BUFFER_LENGTH);
            n = read(sock, buffer, BUFFER_LENGTH - 1); // Leave at least one character for 0 which is used for denoting end of string.
            if (n < 0) {
                fprintf(stderr, "Reading from socket failed. Terminating current connection. ERROR CODE: %d\n", errno);
                break;
            } else if (n == 0) { // client has closed connection
                break;
            } else {
                if (n = parseHTTP(buffer, request)) {
                    fprintf(stderr, "Invalid HTTP request. Terminating current connection. ERROR CODE: %d. Request is:\n%s\n", n, buffer);
                    break;
                } else {
                    response = handleRequest(store, request);
                    n = write(sock, response.c_str(), response.length());
                    if (n < 0) {
                        fprintf(stderr, "Responding to socket failed. Terminating current connection.\n");
                        break;
                    }
                }
            }
        }
        close(sock);
        std::chrono::time_point<std::chrono::high_resolution_clock> endTime = std::chrono::high_resolution_clock::now();
        pthread_mutex_lock(&stat_record_lock);
        std::chrono::duration<float, std::milli> diff = endTime - arriveTime;
        requestTimes.push_back(diff.count()); // Record how many milliseconds have elapsed.
        pthread_mutex_unlock(&stat_record_lock);
    }
}

void *ThreadPoolServer::questHandlerStarter(void *obj) {
    ((ThreadPoolServer *) obj)->questHandler();
}

} // namespace multicore
