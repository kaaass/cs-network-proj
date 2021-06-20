#include "Server.h"
#include "../client/Client.h"
#include "../util/StringUtil.h"
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
    // threadPool->lists()[0]->join();
}

void Server::kill() {
    // 关闭服务器
    active = false;
    threadPool->kill();
}

void Server::runRepl() {
    // 欢迎消息
    std::string welcome =
            "KChat - Simple p2p chat application\n"
            "Version: v1.0\n"
            "用法：使用 link <地址> <端口> 指令建立连接他人";
    std::cout << welcome << std::endl;
    std::cout << "他人连接指令：link " << address << " " << port << std::endl;

    std::string command;
    while (!clientAttached) {
        // 读入指令
        std::getline(std::cin, command);
        auto cmds = StringUtil::split(command, " ");
        // 解析
        if (!cmds.empty()) {
            if (cmds[0] == "link") {
                std::cout << "正在连接 " << cmds[1] << ":" << cmds[2] << "..." << std::endl;
                // 输入端口就连接客户端
                Client::INSTANCE = std::make_unique<Client>();
                auto &client = *Client::INSTANCE;
                // 设置参数
                client.srvAddress = cmds[1];
                client.srvPort = std::stoi(cmds[2]);
                // 初始化客户端
                client.init();
                client.connect();
                // 发起连接
                auto sPort = std::to_string(port);
                const char *cmd[] = {"link", address.c_str(), sPort.c_str()};
                client.runCommand(3, const_cast<char **>(cmd));
                // 启动客户端
                clientAttached = true;
                continue;
            } else if (command == "start") {
                if (!clientAttached) {
                    std::cout << "无法开启会话！当前没有连接的对方" << std::endl;
                }
                continue;
            }
        }
        // 未知指令
        std::cout << "未知指令！仅可以使用 link <地址> <端口> 建立连接！" << std::endl;
    }
    Client::INSTANCE->runRepl();
}

ServerThread::ServerThread(const std::shared_ptr<Epoll> &epoll)
        : EpollEventReceiverThread(epoll) {
    localReadBuffer = std::make_shared<ByteBuffer>();
    localReadBuffer->allocate(READ_BUFFER_SIZE);
}
