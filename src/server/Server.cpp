#include "Server.h"
#include <glog/logging.h>

#define READ_BUFFER_SIZE (1024 * 64)

std::unique_ptr<Server> Server::INSTANCE;
std::shared_ptr<Epoll> Server::epoll;

bool ServerSocketListener::onEvent(const EventContext &context, uint32_t events) {
    // 收到新连接
    while (true) {
        std::shared_ptr<Socket> connSocket = srvSocket->accept();
        if (!connSocket) {
            break;
        }
        LOG(INFO) << "Accept connection from " << connSocket->getAddress() << ":" << connSocket->getPort();
        connSocket->setNonBlocking();

        // epoll 侦听连接
        if (!epoll->attach<ConnectionListener>(EPOLLIN | EPOLLET | EPOLLONESHOT, connSocket)) {
            PLOG(ERROR) << "Cannot add connection to poll";
            continue;
        }
    }
    return true;
}

bool ConnectionListener::onEvent(const EventContext &context, uint32_t events) {
    // 会话处理
    auto *thread = dynamic_cast<ServerThread *>(context.thread);
    session.attachBuffer(thread->localReadBuffer);
    session.handle(context);
    session.detachBuffer();
    // 重置 EPOLLONESHOT 事件继续监听
    switch (session.getStatus()) {
        case READING:
            if (!modifyTo(Server::epoll.get(), EPOLLIN | EPOLLET | EPOLLONESHOT)) {
                PLOG(ERROR) << "Cannot modify fd " << connSocket->fd();
            }
            break;
        case WRITING:
            if (!modifyTo(Server::epoll.get(), EPOLLOUT | EPOLLET | EPOLLONESHOT)) {
                PLOG(ERROR) << "Cannot modify fd " << connSocket->fd();
            }
            break;
        case END:
            // 释放
            return false;
    }
    return true;
}

void Server::init() {
    // 创建服务器 Socket
    std::shared_ptr<Socket> srvSocket = Socket::listen(port, maxListenQueue);

    if (!srvSocket) {
        exit(1);
    }
    srvSocket->setNonBlocking();

    // 初始化 Epoll
    epoll = std::make_shared<Epoll>();

    LOG(INFO) << "Initialize epoll";
    if (!epoll->init()) {
        PLOG(FATAL) << "Cannot init epoll";
        exit(1);
    }

    // 向 Epoll 增加 Socket FD
    LOG(INFO) << "Add server socket to epoll";
    if (!epoll->attach<ServerSocketListener>(EPOLLIN | EPOLLET, srvSocket, epoll)) {
        PLOG(FATAL) << "Cannot add server socket to epoll";
        exit(1);
    }

    // 创建线程池
    threadPool = std::make_unique<ThreadPool<ServerThread>>("SeverThread", threadNum, epoll);
}

void Server::start() {
    // 启动线程池
    if (!threadPool->start()) {
        exit(1);
    }
    // 合入第一个线程
    threadPool->lists()[0]->join();
}

ServerThread::ServerThread(const std::shared_ptr<Epoll> &epoll)
        : EpollEventReceiverThread(epoll) {
    localReadBuffer = std::make_shared<ByteBuffer>();
    localReadBuffer->allocate(READ_BUFFER_SIZE);
}
