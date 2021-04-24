#include <glog/logging.h>
#include "Client.h"

#define CLIENT_RECV_SIZE (10 * 1048576u)

void Client::init() {
    // 打开接收缓冲
    readBuffer.allocate(CLIENT_RECV_SIZE);
}

void Client::start() {
    // 连接服务器
    socket = Socket::connect(srvAddress, srvPort);
    if (!socket) {
        PLOG(FATAL) << "Cannot connect to server";
        exit(1);
    }

    // 欢迎消息
    std::string welcome =
            "SiFtp - Simple multi-thread Ftp client\n"
            "Version: v1.0\n";
    std::cout << welcome << std::endl;

    // 指令执行循环
    std::string command;
    while (active) {
        fprintf(stdout, "> ");
        fflush(stdout);
        // 读入键盘输入
        std::getline(std::cin, command);
        LOG(INFO) << "Read command: '" << command << "'";
        // 发送指令
        sendCommand(command);
        // 处理返回
        handleResponse();
        // 处理退出
        if (command == "kill")
            active = false;
    }
}

bool Client::sendCommand(const std::string &command) const {
    return sendRequest(COMMAND, ByteBuffer::str(command));
}

bool Client::sendRequest(RequestType type, const ByteBuffer &buffer) const {
    ByteBuffer sendBuffer;
    // Write size
    uint32_t packetSize = 1 + buffer.size();
    sendBuffer.write(packetSize);
    // Write type
    sendBuffer.push_back(type);
    // Write data
    sendBuffer = sendBuffer + buffer;
    size_t n = socket->write(reinterpret_cast<const char *>(sendBuffer.data()), sendBuffer.size());
    // check
    if (n != sendBuffer.size()) {
        LOG(WARNING) << "Cannot send request, req = " << sendBuffer.size() << ", n = " << n;
        return false;
    }
    return true;
}

void Client::handleResponse() {
    while (true) {
        // 接收包
        std::shared_ptr<ResponsePacket> resp = receiveResponse();
        if (resp == nullptr)
            return;
        // 判断
        std::string text;
        switch (resp->type) {
            case PLAIN_TEXT:
                // 普通文本，直接打印
                LOG(INFO) << "Got plain text response: " << resp->data;
                resp->data.write((uint16_t) 0);
                printf("%s", reinterpret_cast<const char *>(resp->data.data()));
                return;
            default:
                LOG(ERROR) << "Packet type unknown";
                printf("Packet type unknown");
                return;
        }
    }
}

std::shared_ptr<ResponsePacket> Client::receiveResponse() {
    // 读入长度
    socket->read(readBuffer, 4);
    uint32_t packetLen = readBuffer.readUInt(0);
    // 读入包
    ssize_t readLen = socket->read(readBuffer, packetLen);
    if (readLen <= 0) {
        if (readLen == -1)
            active = false;
        return nullptr;
    }
    // 返回包
    std::shared_ptr<ResponsePacket> resp = std::make_shared<ResponsePacket>();
    resp->type = readBuffer[0];
    resp->data = readBuffer.slice(1, readLen);
    return resp;
}
