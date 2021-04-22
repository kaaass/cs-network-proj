#include "Socket.h"
#include <glog/logging.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <libnet.h>

Socket::~Socket() {
    if (fdSocket != -1) {
        close(fdSocket);
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
