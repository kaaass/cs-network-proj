#ifndef CS_NETWORK_CLIENT_H
#define CS_NETWORK_CLIENT_H


#include <string>
#include <memory>
#include "../network/Socket.h"
#include "../protocol/Protocol.h"

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

    bool sendCommand(const std::string &command) const;

    bool sendRequest(RequestType type, const ByteBuffer &buffer) const;

    std::shared_ptr<ResponsePacket> receiveResponse();

private:

    std::shared_ptr<Socket> socket;
    bool active = true;
    ByteBuffer readBuffer;

public:
    std::string srvAddress;
    uint16_t srvPort;
};


#endif //CS_NETWORK_CLIENT_H
