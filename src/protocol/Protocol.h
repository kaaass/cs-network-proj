#ifndef CS_NETWORK_PROTOCOL_H
#define CS_NETWORK_PROTOCOL_H

#include <memory>
#include "../util/ByteBuffer.h"
#include "../network/Socket.h"

// 请求包类型
enum RequestType {
    COMMAND = 0,
};

// 回复包类型
enum ResponseType {
    PLAIN_TEXT = 0,
};

// 回复包
struct ResponsePacket {
    Byte type;
    ByteBuffer data;
};


#endif //CS_NETWORK_PROTOCOL_H
