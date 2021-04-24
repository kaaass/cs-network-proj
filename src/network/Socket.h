#ifndef CS_NETWORK_SOCKET_H
#define CS_NETWORK_SOCKET_H


#include <utility>
#include <cstdint>
#include <memory>
#include <netinet/in.h>
#include "../util/ByteBuffer.h"

/**
 * 套接字通用类（包括服务器 Socket）
 */
class Socket {

private:

    explicit Socket(int fdSocket) : fdSocket(fdSocket) {}

    // disable copy cons
    Socket(const Socket &other) {}

    // disable copy assignment
    Socket &operator=(const Socket &other) {
        return *this = Socket(other);
    }

public:

    Socket(Socket &&other) noexcept {
        *this = std::move(other);
    }

    Socket &operator=(Socket &&other) noexcept {
        fdSocket = other.fdSocket;
        other.fdSocket = -1;
        return *this;
    }

    void setNonBlocking();

    std::shared_ptr<Socket> accept();

    std::string getAddress() const;

    uint16_t getPort() const;

    ssize_t read(ByteBuffer &buffer, ssize_t limit = 0) const;

    size_t write(const char *buf, size_t n) const;

    /**
     * 创建服务端 socket
     */
    static std::shared_ptr<Socket> listen(uint16_t port, int maxConnections);

    /**
     * 创建客户端 socket
     */
    static std::shared_ptr<Socket> connect(const std::string &address, uint16_t port);

    virtual ~Socket();

    int fd() const { return fdSocket; }

private:
    int fdSocket = -1;
    sockaddr_in address;
};


#endif //CS_NETWORK_SOCKET_H
