#include "File.h"
#include <unistd.h>

#include <string>
#include <utility>

File::~File() {
    if (file != nullptr) {
        fclose(file);
    }
}

File::File(FILE *file) : file(file) {}

std::shared_ptr<File> File::open(const std::string &path, const std::string &mode) {
    FILE *file = fopen(path.c_str(), mode.c_str());
    if (file == nullptr) {
        return nullptr;
    }
    return std::make_shared<File>(file, path);
}

size_t File::getSize() {
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0L, SEEK_SET);
    return fileSize;
}

void File::seek(ssize_t pos) {
    fseek(file, pos, SEEK_SET);
}

int File::getFd() {
    return fileno(file);
}

ssize_t File::write(const ByteBuffer &buffer, ssize_t size) {
    return ::write(getFd(), reinterpret_cast<const void *>(buffer.data()), size);;
}

File::File(FILE *file, std::string path) : file(file), path(std::move(path)) {}

const std::string &File::getPath() const {
    return path;
}

std::string File::getRealPath() const {
    char realPath[4096];
    realpath(path.c_str(), realPath);
    return realPath;
}

std::shared_ptr<File> File::popen(const std::string &command, const std::string &mode) {
    FILE *file = ::popen(command.c_str(), mode.c_str());
    if (file == nullptr) {
        return nullptr;
    }
    return std::make_shared<File>(file, "");
}

ssize_t File::read(ByteBuffer &buffer, ssize_t limit) {
    size_t size = std::min((size_t) limit, buffer.size());
    if (size == 0) {
        size = buffer.size();
    }
    return fread(buffer.data(), 1, size, file);
}
