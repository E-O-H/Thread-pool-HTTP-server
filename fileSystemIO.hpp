#include <string>

namespace multicore {

/**
 * Initialize storage directory.
 *
 * If the directory does not exist, make a new directory;
 * if the directory already exists, wipe it clean.
 *
 * @param fpath the path to the storage directory.
 * @return 0 on success;
 *         -1 on error.
 */
int initDir(const std::string &fpath);

/**
 * Read a key-value file from disk.
 *
 * A key-value file is a file of which the name is the key, and content is the value.
 *
 * @param fpath path (and name) of the file.
 * @param value the argument to return the value of file.
 * @return 0 if file exists;
 *         -1 if file does not exist.
 */
int readFile(const std::string &fpath, std::string &value);

/**
 * Write a key-value file to disk.
 *
 * A key-value file is a file of which the name is the key, and content is the value.
 *
 * @param fpath path (and name) of the file.
 * @param value the value to be written to the file.
 * @return 0 if writing succeed;
 *         -1 if writing failed.
 */
int writeFile(const std::string &fpath, const std::string &value);

/**
 * Delete a key-value file from disk.
 *
 * A key-value file is a file of which the name is the key, and content is the value.
 *
 * @param fpath path (and name) of the file.
 * @return 0 if deleting succeed;
 *         -1 if deleting failed.
 */
int deleteFile(const std::string &fpath);

} // namespace multicore
