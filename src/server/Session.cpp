#include <glog/logging.h>
#include "Session.h"
#include "../protocol/Protocol.h"

#define READ_BUFFER_SIZE (1024 * 64)

void Session::then(SessionStatus pStatus) {
    status = pStatus;
}

void Session::handle(const EventContext &context) {
    if (status == READING) {
        // 允许请求包的读入并回复短包
        // 读入长度
        connSocket->read(readBuffer, 4);
        uint32_t packetLen = readBuffer.readUInt(0);
        // 读入包
        ssize_t readLen = connSocket->read(readBuffer, packetLen);
        if (readLen <= 0) {
            if (readLen == -1)
                then(END);
            return;
        }
        LOG(INFO) << "Read request packet plen = " << packetLen << ", rlen = " << readLen;
        // 分别处理
        Byte type = readBuffer[0];
        switch (type) {
            case COMMAND:
                // 处理指令请求
                processCommand(readLen);
                return;
        }
        // 不符合协议直接关闭
        then(END);
        return;
    } else if (status == WRITING) {
        // 文件写出
    }
}

SessionStatus Session::getStatus() const {
    return status;
}

Session::Session(std::shared_ptr<Socket> connSocket) : connSocket(std::move(connSocket)) {
    readBuffer.allocate(READ_BUFFER_SIZE);
}

void Session::processCommand(uint32_t readLen) {
    // 读入指令
    auto bufCmd = readBuffer.slice(1, readLen);
    bufCmd.write((uint16_t) 0);
    std::string cmd(reinterpret_cast<char *>(bufCmd.data()));
    // 解析指令
    LOG(INFO) << "Read command: " << cmd;
    if (cmd == "kill") {
        // 关闭会话
        say("Bye\n");
        then(END);
        return;
    }
    // Echo
    say("Echo: " + cmd + "\n");
}

void Session::say(const std::string &str) const {
    sendResponse(PLAIN_TEXT, ByteBuffer::str(str));
}

bool Session::sendResponse(ResponseType type, const ByteBuffer &buffer) const {
    // Write type
    Byte bType = type;
    size_t n = connSocket->write(reinterpret_cast<const char *>(&bType), 1);
    // Write resp
    n += connSocket->write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    // check
    if (n != buffer.size() + 1) {
        LOG(WARNING) << "Cannot send response, resp = " << buffer.size() + 1 << ", n = " << n;
        return false;
    }
    return true;
}
