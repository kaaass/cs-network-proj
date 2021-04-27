#include <csignal>
#include <glog/logging.h>
#include "src/server/Server.h"

/**
 * 解析命令行参数
 */
void parseParam(int argc, char **argv) {
    int opt;
    auto &server = *Server::INSTANCE;
    while ((opt = getopt(argc, argv, "p:t:l:")) != -1) {
        switch (opt) {
            case 'p':
                server.port = std::stoi(optarg);
                break;
            case 't':
                server.threadNum = std::stoi(optarg);
                break;
            case 'l':
                FLAGS_minloglevel = std::stoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s [-p port] [-t threads_count] [-l loglevel]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv) {
    // 忽略 SIGPIPE 信号，避免 tcp 连接关闭后 write 直接退出程序
    signal(SIGPIPE, SIG_IGN);

    // 创建服务器
    Server::INSTANCE = std::make_unique<Server>();
    auto &server = *Server::INSTANCE;

    // 初始化参数
    server.threadNum = 10;
    server.port = 8000;
    server.maxListenQueue = 50;
    FLAGS_alsologtostderr = true;
    FLAGS_minloglevel = 0;

    // 解析参数
    parseParam(argc, argv);

    // 初始化日志
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // 初始化服务器
    server.init();

    // 启动服务器
    server.start();

    return 0;
}
