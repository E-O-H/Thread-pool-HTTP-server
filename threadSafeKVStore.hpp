#ifndef _THREADSAFEKVSTORE_H_
#define _THREADSAFEKVSTORE_H_

#include <string>

using std::string;

namespace multicore {

// The class for inner storage. Content is hidden from user.
class ThreadSafeKVStoreImpl;

/**
 * @author Chenyang Tang <ct1856@nyu.edu>
 *
 * @section DESCRIPTION
 *
 * A thread-safe Key-Value storage class, using unordered_map as underlying storage.
 *
 * There are three methods: insert, lookup, and remove. lookup can run simultaneously on multiple
 * threads, while insert or remove will block any other thread from doing any reading or writing
 * while it is running.
 */
class ThreadSafeKVStore {
  public:
    /**
     * Constructor. Makes a new empty storage.
     *
     * @param storagePath the path of storage directory.
     * @param cacheSize the maximum size of the in memory cache.
     */
    ThreadSafeKVStore(string storagePath, unsigned int cacheSize);

    /**
     * Destructor. Will write all memory cache back to disk before destroying them.
     */
    ~ThreadSafeKVStore();

    /**
     * Write all cache in memory to disk.
     *
     * This method is called automatically by destructor,
     * but can also be manually called at any time.
     *
     * @return 0 on success;
     *         -1 on failure.
     */
    int cacheWriteBack();

    /**
     * Insert a key-value pair if the key doesn't exist, or update the value if it does.
     *
     * @param key the key to be inserted.
     * @param value the value to be associated with the key.
     * @return 0 if successful
     *         -1 if there is some fatal error
     */
    int insert(const string &key, const string &value);

    /**
     * Look up a key and write its associated value to the second argument if it exists.
     *
     * @param key the key to be looked up.
     * @param value the variable used to return the associated value.
     * @return 0 if the key is present
     *         -1 if not present
     */
    int lookup(const string &key, string &value);

    /**
     * Delete a key-value pair according to the key provided. If the key does not exist, nothing is done.
     *
     * @param key the key to be deleted.
     * @param value the string to return the value.
     * @return 0 if successful
     *         -1 if there is some fatal error
     */
    int remove(const string &key);

  private:
    // The Inner storage. Hidden from the user.
    ThreadSafeKVStoreImpl *pImpl_;
};

} // namespace multicore
#endif //_THREADSAFEKVSTORE_H_
