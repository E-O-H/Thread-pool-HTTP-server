#include <pthread.h>
#include <unordered_map>
#include <list>
#include <string>

#include "threadSafeKVStore.hpp"
#include "fileSystemIO.hpp"

namespace multicore {

class ThreadSafeKVStoreImpl {
  public:
    ThreadSafeKVStoreImpl(std::string _storagePath, unsigned int _cacheSize)
        : storagePath(_storagePath), cacheSize( _cacheSize) {
        pthread_rwlock_init(&rw_lock, nullptr);
    }

    ~ThreadSafeKVStoreImpl() {
        pthread_rwlock_destroy(&rw_lock);
    }

    std::unordered_map<string, string> store;
    std::list<string> cacheList;
    const std::string storagePath;
    const unsigned int cacheSize;
    pthread_rwlock_t rw_lock;
};

ThreadSafeKVStore::ThreadSafeKVStore(std::string storagePath, unsigned int cacheSize) {
    pImpl_ = new ThreadSafeKVStoreImpl(storagePath, cacheSize);
}

ThreadSafeKVStore::~ThreadSafeKVStore() {
    // Write all memory cache back to disk.
    if (cacheWriteBack()) {
        fprintf(stderr, "Error on writing to disk. Terminating.\n");
        exit(-1);
    }
    delete pImpl_;
}

int ThreadSafeKVStore::cacheWriteBack() {
    int ret = 0;
    for (auto ele : pImpl_->store) {
        if (writeFile(pImpl_->storagePath + "/" + ele.first, ele.second)) {
            ret = -1;
        }
    }
    return ret;
}

int ThreadSafeKVStore::insert(const string &key, const string &value) {
    try {
        pthread_rwlock_wrlock(&pImpl_->rw_lock);
        if (pImpl_->store.count(key)) { // key exists in cache
            pImpl_->cacheList.remove(key);
            pImpl_->cacheList.push_back(key);
            pImpl_->store[key] = value;
        } else { // key does not exist in cache
            pImpl_->cacheList.push_back(key);
            pImpl_->store[key] = value;
            if (pImpl_->cacheList.size() > pImpl_->cacheSize) { // cache is full
                // pop one item in cache and write it back to disk.
                if (writeFile(pImpl_->storagePath + "/" + pImpl_->cacheList.front(),
                              pImpl_->store[pImpl_->cacheList.front()])) {
                    fprintf(stderr, "Error on writing to disk. Terminating.\n");
                    exit(-1);
                }
                pImpl_->store.erase(pImpl_->cacheList.front());
                pImpl_->cacheList.pop_front();
            }
        }
        pthread_rwlock_unlock(&pImpl_->rw_lock);
    } catch(...) {
        return -1;
    }
    return 0;
}

int ThreadSafeKVStore::lookup(const string &key, string &value) {
    bool found = false;
    pthread_rwlock_rdlock(&pImpl_->rw_lock);
    if (pImpl_->store.find(key) != pImpl_->store.end()) { // key already in cache
        found = true;
        value = pImpl_->store.at(key);
        pImpl_->cacheList.remove(key);
        pImpl_->cacheList.push_back(key);
    } else if (!readFile(pImpl_->storagePath + "/" + key, value)) { // key not in cache but on disk
        found = true;
        if (pImpl_->cacheSize) { // cache is not disabled
            if (pImpl_->cacheList.size() == pImpl_->cacheSize) { // cache is full
                // pop one item in cache and write it back to disk.
                if (writeFile(pImpl_->storagePath + "/" + pImpl_->cacheList.front(),
                              pImpl_->store[pImpl_->cacheList.front()])) {
                    fprintf(stderr, "Error on writing to disk. Terminating.\n");
                    exit(-1);
                }
                pImpl_->store.erase(pImpl_->cacheList.front());
                pImpl_->cacheList.pop_front();
            }
            pImpl_->store[key] = value;
            pImpl_->cacheList.push_back(key);
        }
    }
    pthread_rwlock_unlock(&pImpl_->rw_lock);
    return found ? 0 : -1;
}

int ThreadSafeKVStore::remove(const string &key) {
    try {
        pthread_rwlock_wrlock(&pImpl_->rw_lock);
        pImpl_->store.erase(key);
        pImpl_->cacheList.remove(key);
        deleteFile(pImpl_->storagePath + "/" + key);
        pthread_rwlock_unlock(&pImpl_->rw_lock);
    } catch(...) {
        return -1;
    }
    return 0;
}

} // namespace multicore
