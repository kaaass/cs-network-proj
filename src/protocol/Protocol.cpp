#include "Protocol.h"
#include <glog/logging.h>

bool ProtocolHelper::sendRequest(std::shared_ptr<Socket> &socket, RequestType type, const ByteBuffer &buffer) {
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

std::shared_ptr<ResponsePacket> ProtocolHelper::receiveResponse(std::shared_ptr<Socket> &socket, ByteBuffer &buffer) {
    // 读入长度
    socket->read(buffer, 4);
    uint32_t packetLen = buffer.readUInt(0);
    // 读入包
    ssize_t readLen = socket->read(buffer, packetLen);
    if (readLen <= 0) {
        return nullptr;
    }
    // 返回包
    std::shared_ptr<ResponsePacket> resp = std::make_shared<ResponsePacket>();
    resp->type = buffer[0];
    resp->data = buffer.slice(1, readLen);
    return resp;
}
