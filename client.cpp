#include <csignal>
#include <glog/logging.h>

int main(int argc, char **argv) {
    // 初始化日志
    FLAGS_alsologtostderr = true;
    FLAGS_minloglevel = 0;
    google::InitGoogleLogging(argv[0]);
    google::InstallFailureSignalHandler();

    // 忽略 SIGPIPE 信号，避免 tcp 连接关闭后 write 直接退出程序
    signal(SIGPIPE, SIG_IGN);


    return 0;
}
