/**
 * @file packet_dump.cpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-05-05
 *
 * @copyright Copyright (c) 2022
 *
 */


#include "Layer2/layer2.hpp"
#include "net.hpp"
#include "tcpconst.hpp"

#include <iostream>

 /* function prototype declaration */
void dumpARPPacket(ARPHeader *arp_header, uint32_t packet_size);

/* Implement below function to print all necessary headers
 * of the packet including :
 * Ethernet Hdr
 * ARP hdr
 * IP Hdr
 * For Unknown payload type (application data) , just
 * print the offset and size of payload in the frame.
 *
 * We shall be using below API to verify our code changes
 * are correct or not for catching early bugs !!
 * */
void dumpPacket(EthernetHeader *ethernet_header, uint32_t packet_size)
{
    std::cout << "=== begin of " << __FUNCTION__ << " ===" << std::endl;

    std::cout << "Ethernet header :" << std::endl;
    std::cout << " * Destination MAC Address : " << static_cast<std::string>(ethernet_header->dst_mac) << std::endl;
    std::cout << " * Source MAC Address      : " << static_cast<std::string>(ethernet_header->src_mac) << std::endl;

    uint16_t type;
    uint8_t *payload;
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); p) {
        VLANEthernetHeader *vlan_ethernet_header = reinterpret_cast<VLANEthernetHeader *>(ethernet_header);
        std::cout << " * VLAN ID                 : " << vlan_ethernet_header->vlan_8021q_header.getVLANID() << std::endl;
        type = vlan_ethernet_header->type;
        payload = vlan_ethernet_header->payload;
    }
    else {
        type = ethernet_header->type;
        payload = ethernet_header->payload;
    }

    std::cout << " * Ethernet Type           : " << type << std::endl;

    switch (type) {
    case ARP_MSG:
    {
        dumpARPPacket(reinterpret_cast<ARPHeader *>(payload), packet_size - getEthernetHeaderSizeExcludingPayload(ethernet_header));
        break;
    }
    /* TBD!
    case IP_MSG:
    {
        dumpIPPacket(reinterprt_cast<IPHeader *>(ethernet_header->payload), packet_size - getEthernetHeaderSizeExcludingPayload(ethernet_header));
        break;
    }
    */
    default:
    {
        std::cout << " * Payload Size            : " << packet_size - getEthernetHeaderSizeExcludingPayload(ethernet_header) << std::endl;
        break;
    }
    }

    std::cout << "===  end  of " << __FUNCTION__ << " ===" << std::endl;
}

void dumpARPPacket(ARPHeader *arp_header, uint32_t packet_size)
{
    std::cout << std::endl;
    std::cout << "ARP header :" << std::endl;
    std::cout << " * hw_type                 : " << arp_header->hw_type << std::endl;
    std::cout << " * proto_type              : " << arp_header->proto_type << std::endl;
    std::cout << " * hw_addr_len             : " << (int)arp_header->hw_addr_len << std::endl;
    std::cout << " * proto_addr_len          : " << (int)arp_header->proto_addr_len << std::endl;
    std::cout << " * op_code                 : " << arp_header->op_code << " " << (arp_header->op_code == ARP_BROAD_REQ ? "(ARP Broadcast MSG)" : "(ARP Reply)") << std::endl;
    std::cout << " * Source MAC Address      : " << static_cast<std::string>(arp_header->src_mac) << std::endl;
    std::cout << " * Source IP Address       : " << static_cast<std::string>(IPAddress(arp_header->src_ip)) << std::endl;
    std::cout << " * Destination MAC Address : " << static_cast<std::string>(arp_header->dst_mac) << std::endl;
    std::cout << " * Destination IP Address  : " << static_cast<std::string>(IPAddress(arp_header->dst_ip)) << std::endl;
}
