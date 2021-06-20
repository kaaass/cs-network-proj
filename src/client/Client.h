#ifndef CS_NETWORK_CLIENT_H
#define CS_NETWORK_CLIENT_H


#include <string>
#include <memory>
#include <utility>
#include "../network/Socket.h"
#include "../protocol/Protocol.h"
#include "../util/File.h"
#include "DownloadManager.h"

/**
 * 客户端类
 */
class Client {

public:
    // 初始化
    void init();

    // 连接服务器
    void connect();

    // 运行 repl 模式
    void runRepl();

    // 运行指令
    void runCommand(int argc, char **argv);

private:

    // 处理服务器返回
    void handleResponse();

    bool sendCommand(const std::string &command);

    void handleDownloadInfo(const ByteBuffer &response);

private:

    // 连接服务器的套接字
    std::shared_ptr<Socket> socket;
    // 连接活跃
    bool active = true;
    // 读入缓冲区
    ByteBuffer readBuffer;
    // 下载管理
    DownloadManager downloadManager;
    // 目标文件
    std::shared_ptr<File> destFile;
    // 源文件路径
    std::string srcFile;

public:
    // 服务器地址
    std::string srvAddress;
    // 服务器端口
    uint16_t srvPort;
    // 下载线程数
    int downloadThreads;
    // 要传输的文件
    std::string fileToDownload;
    // 保存路径
    std::string savePath;
    // 自动下载
    bool nextDownload = false;

    static std::unique_ptr<Client> INSTANCE;
};


#endif //CS_NETWORK_CLIENT_H
