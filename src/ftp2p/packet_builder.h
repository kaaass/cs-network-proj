#ifndef CS_NETWORK_PACKET_BUILDER_H
#define CS_NETWORK_PACKET_BUILDER_H

#include <util/ByteBuffer.h>
#include "protocol.h"

namespace ftp2p {
    /**
     * 构建协议数据包
     */
    class PacketBuilder {
    public:

        PacketBuilder() = default;

        // 构造最终包数据
        void build(ByteBuffer& buffer);

        // 设置状态码
        PacketBuilder& status(uint32_t status);

        // 处理入网包
        PacketBuilder& pack(const ProtJoin& packet);

        // 处理入网回复包
        PacketBuilder& pack(const ProtJoinReply& packet);

    private:
        ProtHeader header;
        nlohmann::json extra;
    };
}

#endif //CS_NETWORK_PACKET_BUILDER_H
