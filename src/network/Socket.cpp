#include "Socket.h"
#include "../util/rio.h"
#include <glog/logging.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <libnet.h>

#define READ_UNIT 256

Socket::~Socket() {
    if (fdSocket != -1) {
        close(fdSocket);
        LOG(INFO) << "Close socket(" << fdSocket << ") from " << getAddress() << ":" << getPort();
    }
}

std::shared_ptr<Socket> Socket::listen(uint16_t port, int maxConnections) {
    sockaddr_in socketAddress{
            .sin_family = AF_INET,
            .sin_port = htons(port),
            .sin_addr = {
                    .s_addr = INADDR_ANY
            }
    };
    LOG(INFO) << "Listen to 0.0.0.0:" << port;
    // 创建 Socket
    int fdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdSocket < 0) {
        LOG(ERROR) << "Cannot open socket";
        return nullptr;
    }
    // 绑定地址 0.0.0.0
    if (bind(fdSocket, (sockaddr *) &socketAddress, sizeof(socketAddress)) != 0) {
        LOG(ERROR) << "Cannot bind socket to address";
        return nullptr;
    }
    // 侦听
    if (::listen(fdSocket, maxConnections) < 0) {
        LOG(ERROR) << "Cannot listen to 0.0.0.0:" << port;
        return nullptr;
    }
    std::shared_ptr<Socket> result;
    result.reset(new Socket(fdSocket));
    result->address = socketAddress;
    return result;
}

void Socket::setNonBlocking() {
    int oldOption = fcntl(fdSocket, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fdSocket, F_SETFL, newOption);
}

std::shared_ptr<Socket> Socket::accept() {
    sockaddr_in peerAddress{};
    socklen_t peerAddressLen = sizeof(peerAddress);
    int fdConnection = ::accept(fdSocket, (sockaddr *) &peerAddress, &peerAddressLen);
    if (fdConnection < 0) {
        if (errno == EAGAIN | errno == EWOULDBLOCK) {
            return nullptr;
        } else {
            PLOG(ERROR) << "Cannot accept";
            perror("accept");
            return nullptr;
        }
    }
    std::shared_ptr<Socket> result;
    result.reset(new Socket(fdConnection));
    result->address = peerAddress;
    return result;
}

std::string Socket::getAddress() const {
    return std::move(std::string(inet_ntoa(address.sin_addr)));
}

uint16_t Socket::getPort() const {
    return ntohs(address.sin_port);
}

std::shared_ptr<Socket> Socket::connect(const std::string &address, uint16_t port) {
    sockaddr_in socketAddress{
            .sin_family = AF_INET,
            .sin_port = htons(port),
    };
    // 解析地址
    if (!inet_aton(address.c_str(), &socketAddress.sin_addr)) {
        PLOG(ERROR) << "Cannot parse address " << address;
        return nullptr;
    }
    // 创建 Socket
    LOG(INFO) << "Connect to " << address << ":" << port;
    int fdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (fdSocket < 0) {
        LOG(ERROR) << "Cannot open socket";
        return nullptr;
    }
    // 侦听
    if (::connect(fdSocket, reinterpret_cast<const sockaddr *>(&socketAddress), sizeof(socketAddress)) < 0) {
        LOG(ERROR) << "Cannot connect to " << address << ":" << port;
        return nullptr;
    }
    std::shared_ptr<Socket> result;
    result.reset(new Socket(fdSocket));
    result->address = socketAddress;
    return result;
}

ssize_t Socket::read(ByteBuffer &buffer, ssize_t limit) const {
    ssize_t sizeRead = 0;
    if (limit == 0) {
        limit = buffer.size();
    }
    limit = std::min(limit, (ssize_t) buffer.size());
    while (true) {
        ssize_t ret = ::read(fdSocket, buffer.data() + sizeRead, std::min(limit, (ssize_t) READ_UNIT));
        if (ret < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 结束读
                break;
            } else {
                PLOG(WARNING) << "Socket read end unexpectedly";
                break;
            }
        } else if (ret == 0) {
            // EOF
            return -1;
        } else {
            sizeRead += ret;
            if (sizeRead >= limit) {
                break;
            }
        }
    }
    return sizeRead;
}

size_t Socket::write(const char *buf, size_t n) const {
    return rio_writen(fdSocket, buf, n);
}
