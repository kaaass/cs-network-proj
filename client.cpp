#include <csignal>
#include <glog/logging.h>
#include "src/client/Client.h"

void exit_normal(int) {
    exit(0);
}

/**
 * 解析命令行参数
 */
void parseParam(int argc, char **argv) {
    int opt;
    auto &client = *Client::INSTANCE;
    while ((opt = getopt(argc, argv, "h:p:t:v")) != -1) {
        switch (opt) {
            case 'h':
                client.srvAddress = optarg;
                break;
            case 'p':
                client.srvPort = std::stoi(optarg);
                break;
            case 't':
                client.downloadThreads = std::stoi(optarg);
                break;
            case 'v':
                FLAGS_alsologtostderr = true;
                FLAGS_minloglevel = 0;
                break;
            default:
                fprintf(stderr, "Usage: %s [-h host_ip] [-p port] [-t threads_count] [-v] [command]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv) {
    // SIGPIPE 信号直接退出
    signal(SIGPIPE, exit_normal);

    // 创建客户端
    Client::INSTANCE = std::make_unique<Client>();
    auto &client = *Client::INSTANCE;

    // 默认参数
    client.srvAddress = "127.0.0.1";
    client.srvPort = 8000;
    client.downloadThreads = 10;
    FLAGS_alsologtostderr = false;
    FLAGS_minloglevel = 3;

    // 解析参数
    parseParam(argc, argv);

    // 初始化日志
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // 初始化客户端
    client.init();
    client.connect();

    if (optind < argc) {
        // 执行单条指令
        client.runCommand(argv[optind]);
    } else {
        // 启动客户端
        client.runRepl();
    }

    return 0;
}
