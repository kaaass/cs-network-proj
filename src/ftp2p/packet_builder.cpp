#include "packet_builder.h"
#include "Serializer.h"

using namespace ftp2p;

void PacketBuilder::build(ByteBuffer &buffer) {
    // 序列化 Json
    std::string json_str = extra.dump();
    header.sizeExtra = json_str.size();
    // 写头
    buffer.push_back(&header, sizeof(header));
    // 写附加数据
    buffer.push_back(json_str.data(), json_str.size());
    // TODO 写数据
}

PacketBuilder& PacketBuilder::status(uint32_t status) {
    header.status = status;
    return *this;
}

PacketBuilder &PacketBuilder::pack(const ProtJoin &packet) {
    header.packetType = type::JOIN;
    header.addrSrc = address::LOCAL;
    header.addrDest = address::LINK_PEER;
    extra = packet;
    return *this;
}

PacketBuilder &PacketBuilder::pack(const ProtJoinReply &packet) {
    header.packetType = type::JOIN | type::REPLY;
    header.addrSrc = address::LOCAL;
    header.addrDest = packet.addrAllocated;
    extra = packet;
    return *this;
}
