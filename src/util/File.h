#ifndef CS_NETWORK_FILE_H
#define CS_NETWORK_FILE_H

#include <cstdio>
#include <memory>
#include "ByteBuffer.h"

/**
 * 文件管理
 */
class File {
public:
    explicit File(FILE *file);

    File(FILE *file, const std::string &path);

    virtual ~File();

    size_t getSize();

    void seek(ssize_t pos);

    int getFd();

    ssize_t write(const ByteBuffer &buffer, ssize_t size);

    static std::shared_ptr<File> open(const std::string &path, const std::string &mode);

    const std::string &getPath() const;

    std::string getRealPath() const;

private:
    FILE *file = nullptr;
    std::string path;
};


#endif //CS_NETWORK_FILE_H
