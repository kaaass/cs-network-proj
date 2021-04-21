#include <json.hpp>
#include "protocol.h"

#ifndef CS_NETWORK_SERIALIZER_H
#define CS_NETWORK_SERIALIZER_H

namespace ftp2p {
    void to_json(nlohmann::json &j, const ProtJoin &packet);

    void from_json(const nlohmann::json& j, ProtJoin& packet);

    void to_json(nlohmann::json &j, const ProtJoinReply &packet);

    void from_json(const nlohmann::json& j, ProtJoinReply& packet);
}

#endif //CS_NETWORK_SERIALIZER_H
