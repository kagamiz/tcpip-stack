/**
 * @file layer2.hpp
 * @author Jayson Sho Toma
 * @brief defines layer 2 related implementation
 * @version 0.1
 * @date 2022-05-04
 */

#pragma once

#include <cstdint>
#include <cstring>

#include "../graph.hpp"
#include "../net.hpp"

#pragma pack(push,1)
struct EthernetHeader {
    MACAddress dst_mac;
    MACAddress src_mac;
    uint16_t type;
    uint8_t payload[248]; /* Max allowed : 1500 */
    uint32_t FCS;
};
#pragma pack(pop)

#define ETH_HDR_SIZE_EXCL_PAYLOAD       (sizeof(EthernetHeader) - sizeof(EthernetHeader::payload))
#define ETH_FCS(eth_hdr_ptr, payload_size) ( *(uint32_t *)((char *)((EthernetHeader *)eth_hdr_ptr)->payload + payload_size) )

static inline EthernetHeader *ALLOC_ETH_HEADER_WITH_PAYLOAD(char *packet, uint32_t packet_size)
{
    char *head_position = packet - ETH_HDR_SIZE_EXCL_PAYLOAD;
    char *iterator = head_position;
    // dst_mac
    memset(iterator, 0, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    // src_mac
    memset(iterator, 0, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    // type
    memset(iterator, 0, sizeof(uint16_t));
    iterator += sizeof(uint16_t);
    // payload
    memmove(iterator, packet, packet_size);
    iterator += packet_size;
    // FCS
    memset(iterator, 0, sizeof(uint32_t));

    return (EthernetHeader *)head_position;
}

static inline bool l2FrameRecvQualifyOnInterface(Interface *intf, EthernetHeader *ethernet_hdr)
{
    if (!intf->isL3Mode()) {
        return false;
    }

    /* Return true if receiving machine must accept the frame */
    if (intf->getMACAddress() == ethernet_hdr->dst_mac) {
        return true;
    }

    if (ethernet_hdr->dst_mac == MACAddress::BROADCAST_MAC_ADDRESS) {
        return true;
    }

    return false;
}

void layer2FrameRecv(Node *node, Interface *interface, char *packet, uint32_t packet_size);
