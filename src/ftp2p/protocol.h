/**
 * FTP2P 协议
 */

#include <cstdint>
#include <string>
#include <vector>
#include <json.hpp>

#ifndef CS_NETWORK_PROTOCOL_H
#define CS_NETWORK_PROTOCOL_H

namespace ftp2p {

    /**
     * Ftp2p Peer 地址，长度为 4 字节
     */
    typedef uint32_t Address;

    namespace address {
        // 0x0 地址不进行转发，视为本机
        const static uint32_t LOCAL = 0x0u;
        // 0xffffffff 地址不进行转发，视为直连链路的对方
        const static uint32_t LINK_PEER = 0xffffffffu;
    }

    namespace type {
        const static uint8_t REPLY = 0x80u;
        const static uint8_t FORWARD = 0x40u;
        const static uint8_t ROUTE = 0x20u;
        const static uint8_t COMMAND = 0x10u;
        const static uint8_t BUFFER = 0x08u;
        const static uint8_t JOIN = 0x04u;
    }

    namespace status {
        // 成功状态码
        const static uint32_t OK = 0;
    }

    /**
     * Ftp2p 协议头
     */
    struct ProtHeader {
        /**
         * 目标地址
         */
        Address addrSrc = 0u;

        /**
         * 源地址
         */
        Address destSrc = 0u;

        union {
            uint8_t packetType = 0u;
            struct {
                /**
                 * 应答包
                 */
                uint8_t typeReply: 1;

                /**
                 * 转发包：edge <-> supernode <-> edge
                 * 表示包通过转发来到 edge，supernode 在转发时添加此标志
                 */
                uint8_t typeForward: 1;

                /**
                 * 路由包：edge <-> supernode
                 * 用于建立 p2p 信道
                 */
                uint8_t typeRoute: 1;

                /**
                 * 指令包：edge <-...-> edge
                 * 进行文件交换指令，如：LIST
                 */
                uint8_t typeCommand: 1;

                /**
                 * 缓冲区数据交换：edge <-...-> edge
                 * 进行数据交换。Buffer = 1 && Reply = 0，则有 data 段
                 */
                uint8_t typeBuffer: 1;

                /**
                 * 公告包：edge <-> supernode
                 * 告知超级节点节点入网
                 */
                uint8_t typeJoin: 1;
            };
        };

        /**
         * 状态码
         */
        uint8_t status = status::OK;

        /**
         * 附加头信息长度。JSON 文本。
         */
        uint16_t sizeExtra = 0;
    };

    struct ProtData {
        // 数据段长度
        uint16_t size = 0;

        // 数据内容
        uint8_t *buffer = nullptr;
    };

    /**
     * 协议报文
     */
    struct ProtDiagram {};

    /**
     * TODO 入网请求包。当 edge 节点请求加入网络时向网络中的一个 supernode 发送（目前只允许一个 supernode）
     * 包的源地址为 LOCAL 目标地址为 LINK_PEER。Join = 1
     * supernode 接收到后根据 identity 为结点分配网络地址，并增加路由 <edge, sock>
     */
    struct ProtJoin : public ProtDiagram {
        // 身份识别标志，固定不变
        uint64_t identity = 0;
    };

    /**
     * TODO 入网请求回复包。分配相关地址
     * 包的源地址为 LOCAL 目标地址为分配的地址。Join = 1, Reply = 1
     */
    struct ProtJoinReply : public ProtDiagram {
        // 信息/错误消息
        std::string info;
    };

    /**
     * TODO 建立路由请求包。sedge -> supernode。当 sedge 节点没有到目标 dedge 的路由时发送
     * Src = sedge 地址, Dest = dedge 地址, Route = 1
     * supernode 收到后向 dedge 发出建立路由中转包
     */
    struct ProtRoute : public ProtDiagram {
        // sedge 开放的供 dedge 连接的 tcp 端口
        uint8_t tcpPort;
    };

    /**
     * TODO 建立路由中转包。supernode -> dedge。
     * Src = sedge 地址, Dest = dedge 地址, Route = 1, Forward = 1
     * dedge 收到后向 sedge 发出建立路由协商包
     */
    struct ProtRouteForward : public ProtDiagram {
        // sedge 的 IPv4 地址
        uint32_t peerIpv4;
        // sedge 的开放端口
        uint8_t tcpPort;
    };

    /**
     * TODO 建立路由协商包。dedge -> sedge。
     * Src = dedge, Dest = sedge, Route = 1
     * sedge 收到后增加路由 <sedge, sock>，发出回复路由协商包
     */
    struct ProtRouteNegotiate : public ProtDiagram {
    };

    /**
     * TODO 回复路由协商包。sedge -> dedge。
     * Src = dedge, Dest = sedge, Route = 1, Reply = 1
     * dedge 收到增加路由 <dedge, sock>
     */
    struct ProtRouteNegotiateReply : public ProtDiagram {
    };

    /**
     * TODO 指令请求包。sedge <-...-> dedge
     * Src = sedge, Dest = dedge, Command = 1, Forward = Any
     * dedge 执行相关指令，发出指令结果包
     */
    struct ProtCommand : public ProtDiagram {
        // 指令
        std::string command;
        // 指令参数
        nlohmann::json argument;
    };

    /**
     * TODO 指令结果包。dedge <-...-> sedge
     * Src = dedge, Dest = sedge, Command = 1, Forward = Any, Reply = 1
     */
    struct ProtCommandReply : public ProtDiagram {
        // 信息/错误消息
        std::string info;
        // 结果
        nlohmann::json data;
    };

    /**
     * TODO 缓冲区数据包。sedge <-...-> dedge
     * Src = sedge, Dest = dedge, Buffer = 1, Forward = Any
     */
    struct ProtBuffer : public ProtDiagram {
        // TODO
    };

    /**
     * TODO 缓冲区数据响应包。dedge <-...-> sedge
     * Src = sedge, Dest = dedge, Buffer = 1, Forward = Any, Reply = 1
     */
    struct ProtBufferReply : public ProtDiagram {
        // TODO
    };
}

#endif //CS_NETWORK_PROTOCOL_H
