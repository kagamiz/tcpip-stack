/**
 * @file ping.cpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-07
 */

#include "ping.hpp"

#include <iostream>

#include "../tcpconst.hpp"

extern void demotePacketToLayer3(Node *node, char *packet, uint32_t packet_size, int protocol_number, const IPAddress &dest_ip_address);

void layer5PingFunc(Node *node, const std::string &dst_ip_addr)
{
    std::cout << "src node : " << node->getName() << " dst : " << static_cast<std::string>(dst_ip_addr) << std::endl;
    demotePacketToLayer3(node, nullptr, 0, ICMP_PRO, IPAddress(dst_ip_addr));
}
