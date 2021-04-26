#ifndef CS_NETWORK_PROTOCOL_H
#define CS_NETWORK_PROTOCOL_H

#include <memory>
#include "../util/ByteBuffer.h"
#include "../network/Socket.h"

// 请求包类型
enum RequestType {
    COMMAND = 0,
    DOWNLOAD = 1,
    DOWNLOAD_INFO = 2,
};

// 回复包类型
enum ResponseType {
    PLAIN_TEXT = 0,
    BINARY = 1,
    DOWNLOAD_INFO_REPLY = 2,
};

// 回复包
struct ResponsePacket {
    Byte type;
    ByteBuffer data;
};

// 下载请求。之后跟随文件位置字符串
struct DownloadRequestPacket {
    // 文件首部偏移，目前忽略
    uint64_t offset;
    // 下载分块大小，为 0 不限，目前忽略
    uint64_t size;
};

// 下载信息/二进制响应，之后跟随二进制数据
struct DownloadInfoPacket {
    // 附加数据大小
    uint64_t size;
};

class ProtocolHelper {
public:

    // 发送请求包
    static bool sendRequest(std::shared_ptr<Socket> &socket, RequestType type, const ByteBuffer &buffer);

    // 接受响应包
    static std::shared_ptr<ResponsePacket> receiveResponse(std::shared_ptr<Socket> &socket, ByteBuffer &buffer);
};

#endif //CS_NETWORK_PROTOCOL_H
