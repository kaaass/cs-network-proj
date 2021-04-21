#include "Serializer.h"

using namespace ftp2p;

void ftp2p::to_json(nlohmann::json &j, const ProtJoin &packet) {
    j["identity"] = packet.identity;
}

void ftp2p::from_json(const nlohmann::json &j, ProtJoin &packet) {
    packet.identity = j["identity"].get<uint64_t>();
}

void ftp2p::to_json(nlohmann::json &j, const ProtJoinReply &packet) {
    j["info"] = packet.info;
}

void ftp2p::from_json(const nlohmann::json &j, ProtJoinReply &packet) {
    packet.info = j["info"].get<std::string>();
}
