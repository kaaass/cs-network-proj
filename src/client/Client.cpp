#include <glog/logging.h>
#include "Client.h"
#include "../util/StringUtil.h"

// 256M
#define CLIENT_RECV_SIZE (256 * 1048576u)

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
    bool sent;
    while (active) {
        fprintf(stdout, "> ");
        fflush(stdout);
        // 读入键盘输入
        std::getline(std::cin, command);
        LOG(INFO) << "Read command: '" << command << "'";
        // 发送指令
        sent = sendCommand(command);
        if (!sent) {
            printf("Failed to send command\n");
            continue;
        }
        // 处理返回
        handleResponse();
        // 处理退出
        if (command == "kill")
            active = false;
    }
}

bool Client::sendCommand(const std::string &command) {
    auto cmds = StringUtil::split(command, " ");
    if (cmds[0] == "download" || cmds[0] == "dl") {
        // 下载指令
        if (cmds.size() != 3) {
            printf("'download' takes exact 3 params\n");
            return false;
        }
        // 解析
        std::string filepath = std::move(cmds[1]);
        std::string dest = std::move(cmds[2]);
        LOG(INFO) << "Download '" << filepath << "' to '" << dest << "'";
        // 打开本地文件
        auto destFile = File::open(dest, "wb");
        if (!destFile) {
            perror("Cannot open dest file");
            PLOG(INFO) << "Cannot open dest file";
            return false;
        }
        fileReceiveProcess = std::make_unique<FileReceiveProcess>(destFile, 0, 0);
        // 发送下载请求
        DownloadRequestPacket req{
            .offset = 0,
            .size = 0,
        };
        ByteBuffer data(&req, sizeof(req));
        data = data + ByteBuffer::str(filepath);
        bool ret = sendRequest(DOWNLOAD, data);
        // 若失败，释放文件
        if (!ret) {
            fileReceiveProcess = nullptr;
        }
        return ret;
    }
    // 其他指令
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
                LOG(INFO) << "Got plain text response, len = " << resp->data.size();
                resp->data.write((uint16_t) 0);
                printf("%s", reinterpret_cast<const char *>(resp->data.data()));
                return;
            case BINARY:
                // 处理下载包
                handleBinary(resp->data);
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

void Client::handleBinary(const ByteBuffer &response) {
    if (!fileReceiveProcess) {
        printf("Client entered wrong state!");
        LOG(FATAL) << "Client entered wrong state!";
        exit(1);
    }
    // 解析返回包
    BinaryResponsePacket resp = *reinterpret_cast<BinaryResponsePacket const *>(response.data());
    fileReceiveProcess->rest = resp.size;
    fileReceiveProcess->totalSize = resp.size;
    // 接受文件
    auto &rest = fileReceiveProcess->rest;
    ssize_t receive, written;
    while (rest > 0) {
        // 读入数据
        receive = socket->read(readBuffer, std::min(readBuffer.size(), rest));
        if (receive < 0) {
            // 连接关闭
            printf("Server connection closed");
            exit(1);
        }
        // 写入文件
        written = fileReceiveProcess->destFile->write(readBuffer, receive);
        if (receive != written) {
            // 写入错误
            perror("Write count mismatch");
            PLOG(FATAL) << "Write count mismatch receive = " << receive << ", written = " << written;
            exit(1);
        }
        // 减去剩余
        rest -= receive;
    }
    // 完成写入
    printf("Successful write %lu bytes to file %s.\n",
           fileReceiveProcess->totalSize,
           fileReceiveProcess->destFile->getRealPath().c_str());
}
