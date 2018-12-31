#define _XOPEN_SOURCE 500

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>
#include <cstdio>
#include <string>
#include <fstream>
#include <sstream>
using std::string;
using std::fstream;
using std::stringstream;

namespace multicore {

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);

int initDir(const string &fpath) {
    struct stat st;
    if(stat(fpath.c_str(), &st) || !S_ISDIR(st.st_mode)) { // file not found or file is not dir
        return mkdir(fpath.c_str(), S_IRWXU);
    }
    // Dir already exists
    return nftw(fpath.c_str(), unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int readFile(const string &fpath, string &value) {
    fstream file;
    file.open(fpath.c_str(), fstream::in);
    if (file.good()) {
        stringstream buffer;
        buffer << file.rdbuf();
        value = buffer.str();
    }
    return file.good() ? 0 : -1;
}

int writeFile(const string &fpath, const string &value) {
    fstream file;
    file.open(fpath.c_str(), fstream::out | fstream::trunc);
    if (file.good()) {
        file << value;
    }
    return file.good() ? 0 : -1;
}

int deleteFile(const string &fpath) {
    return std::remove(fpath.c_str());
}

static int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    int rv;
    if (ftwbuf->level == 0)
        return 0;
    rv = std::remove(fpath);
    if (rv)
        perror(fpath);
    return rv;
}

} // namespace multicore
