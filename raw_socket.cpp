#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<netinet/ip.h>
#include<netinet/tcp.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <algorithm>

int main() {
    // 创建 Raw Socket
    int sock = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock == -1) {
        perror("Failed to create socket");
        exit(1);
    }

    // 开始读入数据
    auto *buffer = new unsigned char[65536];
    in_addr ip_src_addr{}, ip_dest_addr{};
    int packet_size;

    while (true) {
        // 读入数据
        packet_size = recvfrom(sock, buffer, 65536, 0, nullptr, nullptr);
        if (packet_size == -1) {
            printf("Failed to receive packets\n");
            return 1;
        }

        printf("========== Packet Information ==========\n");

        // 解析 IP 首部
        auto *ip_pdu = (ip *) buffer;

        printf("---------- Network Layer (IP) ----------\n");
        printf("  Src IP:              %15s\n", (char *) inet_ntoa(ip_pdu->ip_src));
        printf("  Dest IP:             %15s\n", (char *) inet_ntoa(ip_pdu->ip_dst));
        printf("  Packet Size (bytes): %15hu\n", ntohs(ip_pdu->ip_len));
        printf("  Identification:      %15hu\n", ntohs(ip_pdu->ip_id));
        printf("  Time To Alive:       %15u\n", ip_pdu->ip_ttl);
        auto ip_off = ntohs(ip_pdu->ip_off);
        printf("  Mark:         RF = %d; DF = %d; MF = %d\n",
               !!(ip_off & IP_RF), !!(ip_off & IP_DF), !!(ip_off & IP_MF));
        printf("  Fragment Offset:     %15u\n", ip_off & IP_OFFMASK);

        // 解析 TCP 首部
        auto *tcp_pdu = (tcphdr *) (buffer + ip_pdu->ip_hl * 4u);

        printf("-------- Transport Layer  (TCP) --------\n");
        printf("  Src port:            %15hu\n", ntohs(tcp_pdu->th_sport));
        printf("  Dest port:           %15hu\n", ntohs(tcp_pdu->th_dport));
        printf("  Seq Number:          %15u\n", ntohl(tcp_pdu->th_seq));
        printf("  Ack Number:          %15u\n", ntohl(tcp_pdu->th_ack));
        printf("  SYN = %d;  ACK = %d;  FIN = %d; RST = %d\n",
               tcp_pdu->syn, tcp_pdu->ack, tcp_pdu->fin, tcp_pdu->rst);

        // 解析应用层数据
        auto *app_payload = buffer + ip_pdu->ip_hl * 4u + tcp_pdu->th_off * 4u;
        size_t app_len = ntohs(ip_pdu->ip_len) - ip_pdu->ip_hl * 4u - tcp_pdu->th_off * 4u;

        printf("---------- Application Layer -----------\n");
        printf("  Size:                %15zu\n", app_len);

        // 打印数据前 8 字节
        if (app_len > 0) {
            printf("  Data (first 8 bytes):\n    ");
            size_t print_len = std::min(size_t(8), app_len);
            for (size_t i = 0; i < print_len; i++) {
                printf("%02x ", app_payload[i]);
            }
        }

        printf("\n========================================\n\n");
    }

    delete[] buffer;
    return 0;
}