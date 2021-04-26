#include <glog/logging.h>
#include "Client.h"
#include "../util/StringUtil.h"

// 1M
#define CLIENT_RECV_SIZE (1 * 1048576u)

std::unique_ptr<Client> Client::INSTANCE;

void Client::init() {
    // 打开接收缓冲
    readBuffer.allocate(CLIENT_RECV_SIZE);
    // 下载管理
    downloadManager.init();
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
    if (cmds.empty()) {
        return false;
    }
    if (cmds[0] == "download" || cmds[0] == "dl") {
        // 下载指令
        if (cmds.size() != 3) {
            printf("download takes exact 2 params\n");
            return false;
        }
        // 解析
        srcFile = std::move(cmds[1]);
        std::string dest = std::move(cmds[2]);
        LOG(INFO) << "Download '" << srcFile << "' to '" << dest << "'";
        // 打开本地文件
        destFile = File::open(dest, "r+b");
        if (!destFile) {
            destFile = File::open(dest, "wb");
        }
        if (!destFile) {
            perror("Cannot open dest file");
            PLOG(INFO) << "Cannot open dest file";
            return false;
        }
        // 请求下载信息
        DownloadRequestPacket req{
            .offset = 0,
            .size = 0,
        };
        ByteBuffer data(&req, sizeof(req));
        data = data + ByteBuffer::str(srcFile);
        bool ret = ProtocolHelper::sendRequest(socket, DOWNLOAD_INFO, data);
        // 若失败，释放文件
        if (!ret) {
            destFile = nullptr;
        }
        return ret;
    }
    // 其他指令
    return ProtocolHelper::sendRequest(socket, COMMAND, ByteBuffer::str(command));
}

void Client::handleResponse() {
    while (true) {
        // 接收包
        std::shared_ptr<ResponsePacket> resp = ProtocolHelper::receiveResponse(socket, readBuffer);
        if (resp == nullptr) {
            // 套接字关闭
            active = false;
            return;
        }
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
                printf("Unexpected binary transferred in command session!");
                LOG(FATAL) << "Unexpected binary transferred in command session!";
                exit(1);
            case DOWNLOAD_INFO_REPLY:
                handleDownloadInfo(resp->data);
                return;
            default:
                LOG(ERROR) << "Packet type unknown";
                printf("Packet type unknown");
                return;
        }
    }
}

void Client::handleDownloadInfo(const ByteBuffer &response) {
    // 解析返回包
    DownloadInfoPacket resp = *reinterpret_cast<DownloadInfoPacket const *>(response.data());
    // 下载
    auto ret = downloadManager.downloadWait(srcFile, destFile, resp.size);
    // 完成写入
    if (ret) {
        printf("Successful write %lu bytes to file %s.\n",
               resp.size,
               destFile->getRealPath().c_str());
    }
    destFile = nullptr;
}
