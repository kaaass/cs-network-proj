#ifndef CS_NETWORK_SERVER_H
#define CS_NETWORK_SERVER_H


#include <utility>

#include "Events.h"
#include "../network/Socket.h"
#include "ThreadPool.hpp"
#include "Session.h"

/**
 * 服务器线程，接收 Epoll 事件
 */
class ServerThread : public EpollEventReceiverThread {
public:
    explicit ServerThread(const std::shared_ptr<Epoll> &epoll);

    friend class ConnectionListener;
private:
    std::shared_ptr<ByteBuffer> localReadBuffer;
};

/**
 * 服务器
 */
class Server {

public:
    // 初始化服务器
    void init();

    // 启动服务器
    void start();

    // 关闭服务器
    void kill();

    void runRepl();

public:
    static std::shared_ptr<Epoll> epoll;
    bool active = true;

    // Config

    // 线程池大小
    int threadNum;
    // 监听端口
    int port;
    // 最大连接队列（accept 执行前允许维持的连接数）
    int maxListenQueue;
    // 服务器地址
    std::string address;

    static std::unique_ptr<Server> INSTANCE;

    bool clientAttached = false;

private:
    std::unique_ptr<ThreadPool<ServerThread>> threadPool;
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
