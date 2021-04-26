#ifndef CS_NETWORK_SESSION_H
#define CS_NETWORK_SESSION_H

#include "../network/Socket.h"
#include "Events.h"
#include "../protocol/Protocol.h"
#include "../util/File.h"
#include <map>

enum SessionStatus {
    READING,
    WRITING,
    END
};

// 传输文件过程
struct FileProcess {
    std::shared_ptr<File> file;
    size_t rest;

    FileProcess(std::shared_ptr<File> file, size_t rest) : file(std::move(file)), rest(rest) {}
};

/**
 * 用户会话
 */
class Session {
public:

    explicit Session(std::shared_ptr<Socket> connSocket);

    void handle(const EventContext &context);

    // 处理指令
    void processCommand(uint32_t readLen);

    // 处理下载请求
    void processDownload(uint32_t readLen);

    // 处理文件写出
    void handleWriting();

    SessionStatus getStatus() const;

private:

    void say(const std::string &str) const;

    bool sendResponse(ResponseType type, const ByteBuffer& buffer) const;

    std::shared_ptr<File> openDownloadFile(const std::string &filepath) const;

    std::string validatePath(const std::string& path) const;

    // 下一步状态
    void then(SessionStatus status);

private:

    // 会话状态
    SessionStatus status = READING;

    // 会话 Socket
    std::shared_ptr<Socket> connSocket;

    // 读入缓冲
    ByteBuffer readBuffer;

    // 文件传输进度
    std::unique_ptr<FileProcess> fileProcess;

};


#endif //CS_NETWORK_SESSION_H
