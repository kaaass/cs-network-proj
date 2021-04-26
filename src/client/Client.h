#ifndef CS_NETWORK_CLIENT_H
#define CS_NETWORK_CLIENT_H


#include <string>
#include <memory>
#include <utility>
#include "../network/Socket.h"
#include "../protocol/Protocol.h"
#include "../util/File.h"

// 文件接受过程
struct FileReceiveProcess {
    FileReceiveProcess(std::shared_ptr<File> destFile, size_t rest, uint64_t size)
            : destFile(std::move(destFile)), rest(rest), totalSize(size) {}

    // 目标文件
    std::shared_ptr<File> destFile;
    // 剩余数量
    size_t rest;
    // 文件总大小
    uint64_t totalSize;
};

/**
 * 客户端类
 */
class Client {

public:
    void init();

    void start();

private:

    // 处理服务器返回
    void handleResponse();

    bool sendCommand(const std::string &command);

    bool sendRequest(RequestType type, const ByteBuffer &buffer) const;

    std::shared_ptr<ResponsePacket> receiveResponse();

    void handleBinary(const ByteBuffer &response);

private:

    // 连接服务器的套接字
    std::shared_ptr<Socket> socket;
    // 连接活跃
    bool active = true;
    // 读入缓冲区
    ByteBuffer readBuffer;
    // 传输过程信息
    std::unique_ptr<FileReceiveProcess> fileReceiveProcess;

public:
    std::string srvAddress;
    uint16_t srvPort;
};


#endif //CS_NETWORK_CLIENT_H
