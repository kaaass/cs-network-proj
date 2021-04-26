#include <csignal>
#include <glog/logging.h>
#include "src/client/Client.h"

void exit_normal(int) {
    exit(0);
}

int main(int argc, char **argv) {
    // 初始化日志
    FLAGS_alsologtostderr = true;
    FLAGS_minloglevel = 0;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // SIGPIPE 信号直接退出
    signal(SIGPIPE, exit_normal);

    // 创建客户端
    Client::INSTANCE = std::make_unique<Client>();
    auto &client = *Client::INSTANCE;
    client.srvAddress = "127.0.0.1";
    client.srvPort = 8000;
    client.downloadThreads = 10;

    // 初始化客户端
    client.init();

    // 启动客户端
    client.start();

    return 0;
}
