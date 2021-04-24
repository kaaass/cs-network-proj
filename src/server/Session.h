#ifndef CS_NETWORK_SESSION_H
#define CS_NETWORK_SESSION_H

#include "../network/Socket.h"
#include "Events.h"
#include "../protocol/Protocol.h"

enum SessionStatus {
    READING,
    WRITING,
    END
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

    SessionStatus getStatus() const;

private:

    void say(const std::string &str) const;

    bool sendResponse(ResponseType type, const ByteBuffer& buffer) const;

    // 下一步状态
    void then(SessionStatus status);

private:

    // 会话状态
    SessionStatus status = READING;

    // 会话 Socket
    std::shared_ptr<Socket> connSocket;

    // 读入缓冲
    ByteBuffer readBuffer;
};


#endif //CS_NETWORK_SESSION_H
