#ifndef CS_NETWORK_SERVER_H
#define CS_NETWORK_SERVER_H


#include <utility>

#include "Events.h"
#include "../network/Socket.h"
#include "../server.h"
#include "ThreadPool.hpp"
#include "Session.h"

/**
 * 服务器
 */
class Server {

public:
    // 初始化服务器
    void init();

    // 启动服务器
    void start();

public:
    static std::shared_ptr<Epoll> epoll;

    // Config

    // 线程池大小
    int threadNum;
    // 监听端口
    int port;
    // 最大连接数
    int maxConn;

private:
    std::unique_ptr<ThreadPool<EpollEventReceiverThread>> threadPool;
};

/**
 * 服务器 socket 处理，接受连接
 */
class ServerSocketListener : public FdEventListener {
public:
    explicit ServerSocketListener(std::shared_ptr<Socket> srvSocket, std::shared_ptr<Epoll> epoll) :
            FdEventListener(srvSocket->fd()), srvSocket(std::move(srvSocket)), epoll(std::move(epoll)) {}

    bool onEvent(const EventContext &context, uint32_t events) override;

private:
    std::shared_ptr<Socket> srvSocket;
    std::shared_ptr<Epoll> epoll;
};

/**
 * 连接 socket 处理，进行服务器逻辑
 */
class ConnectionListener : public FdEventListener {
public:
    explicit ConnectionListener(std::shared_ptr<Socket> connSocket) :
            FdEventListener(connSocket->fd()), connSocket(connSocket), session(connSocket) {}

    bool onEvent(const EventContext &context, uint32_t events) override;

private:
    std::shared_ptr<Socket> connSocket;
    Session session;
};


#endif //CS_NETWORK_SERVER_H
