/**
 * @file layer2.cpp
 * @author Jayson Sho Toma
 * @brief defines layer 2 related implementation
 * @version 0.1
 * @date 2022-05-04
 */

#include "../comm.hpp"
#include "../color.hpp"
#include "../tcpconst.hpp"
#include "layer2.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

 /* extern function prototype declaration */

extern void l2SwitchRecvFrame(Interface *interface, char *packet, uint32_t packet_size);

/* function prototype declaration */
static void processARPReplyMessage(Node *node, Interface *iif, EthernetHeader *ethernet_header);
static void processARPBroadcastRequest(Node *node, Interface *iif, EthernetHeader *ethernet_header);

void layer2FrameRecv(Node *node, Interface *interface, char *packet, uint32_t packet_size)
{
    /* Entry point into TCP/IP from bottom */
    EthernetHeader *ethernet_header = reinterpret_cast<EthernetHeader *>(packet);

    if (!l2FrameRecvQualifyOnInterface(interface, ethernet_header)) {
        std::cout << "L2 Frame Rejected" << std::endl;
        return;
    }

    std::cout << "L2 Frame Accepted" << std::endl;

    if (interface->isL3Mode()) {
        switch (ethernet_header->type) {
        case ARP_MSG:
        {
            ARPHeader *arp_hdr = reinterpret_cast<ARPHeader *>(ethernet_header->payload);
            switch (arp_hdr->op_code) {
            case ARP_BROAD_REQ:
                processARPBroadcastRequest(node, interface, ethernet_header);
                break;
            case ARP_REPLY:
                processARPReplyMessage(node, interface, ethernet_header);
                break;
            default:
                break;
            }
        }
        break;

        default:
            // promotePacketToLayer3(node, interface, packet, packet_size);
            break;
        }
    }
    else if (interface->getL2Mode() == InterfaceNetworkProperty::L2Mode::ACCESS ||
             interface->getL2Mode() == InterfaceNetworkProperty::L2Mode::TRUNK) {
        l2SwitchRecvFrame(interface, packet, packet_size);
    }
}

ARPEntry::ARPEntry() :
    ip_addr(0),
    mac_addr(),
    oif_name("")
{

}

ARPEntry *ARPTable::arpTableLookup(const std::string &ip_addr)
{
    auto result = std::find_if(
        std::begin(arp_table),
        std::end(arp_table),
        [&](const ARPEntry &arp_entry)
        {
            return arp_entry.ip_addr == IPAddress(ip_addr);
        }
    );

    if (result == std::end(arp_table)) {
        return nullptr;
    }
    return &(*result);
}

void ARPTable::deleteEntry(const std::string &ip_addr)
{
    arp_table.remove_if(
        [&](const ARPEntry &arp_entry)
        {
            return arp_entry.ip_addr == IPAddress(ip_addr);
        });
}

bool ARPTable::addEntry(ARPEntry *arp_entry)
{
    ARPEntry *arp_entry_old = arpTableLookup(arp_entry->ip_addr);
    // no need to add!
    if (arp_entry_old && memcmp(arp_entry_old, arp_entry, sizeof(ARPEntry)) == 0) {
        // caller need to free ARPEntry
        return false;
    }

    if (arp_entry_old) {
        deleteEntry(arp_entry_old->ip_addr);
    }

    arp_table.push_back(*arp_entry);

    return true;
}

void ARPTable::updateFromARPReply(ARPHeader *arp_header, Interface *iif)
{
    assert(arp_header->op_code == ARP_REPLY);

    ARPEntry arp_entry;
    arp_entry.ip_addr = IPAddress(arp_header->src_ip);
    arp_entry.mac_addr = arp_header->src_mac;
    arp_entry.oif_name = iif->getName();

    addEntry(&arp_entry);
}

void ARPTable::dump() const
{
    for (const auto &arp_entry : arp_table) {
        std::cout <<
            "IP : " <<
            getColoredString(arp_entry.ip_addr, "Light Red") <<
            ", MAC : " <<
            static_cast<std::string>(arp_entry.mac_addr) <<
            ", OIF = " <<
            arp_entry.oif_name <<
            std::endl;
    }
}

ARPTable *getNewARPTable()
{
    return ARPTable::getNewTable();
}

void deleteARPTable(ARPTable *arp_table)
{
    delete arp_table;
}

static void sendARPReplyMessage(EthernetHeader *ethernet_header_in, Interface *oif)
{
    ARPHeader *arp_header_in = (ARPHeader *)ethernet_header_in->payload;
    EthernetHeader *ethenet_header_reply = (EthernetHeader *)(new char[MAX_PACKET_BUFFER_SIZE]);

    ethenet_header_reply->dst_mac = ethernet_header_in->src_mac;
    ethenet_header_reply->src_mac = oif->getMACAddress();

    ethenet_header_reply->type = ARP_MSG;
    ARPHeader *arp_header_reply = (ARPHeader *)ethenet_header_reply->payload;

    arp_header_reply->hw_type = 1;
    arp_header_reply->proto_type = 0x0800;
    arp_header_reply->hw_addr_len = sizeof(MACAddress);
    arp_header_reply->proto_addr_len = 4;

    arp_header_reply->op_code = ARP_REPLY;

    arp_header_reply->src_mac = oif->getMACAddress();
    arp_header_reply->src_ip = oif->getIPAddress();

    arp_header_reply->dst_mac = arp_header_in->src_mac;
    arp_header_reply->dst_ip = arp_header_in->src_ip;

    ETH_FCS(ethenet_header_reply, sizeof(ARPHeader)) = 0;

    uint32_t total_packet_size = ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(ARPHeader);
    char *shifted_packet_buffer = packetBufferShiftRight(reinterpret_cast<char *>(ethenet_header_reply), total_packet_size, MAX_PACKET_BUFFER_SIZE);
    oif->sendPacketOut(shifted_packet_buffer, total_packet_size);
    delete[] ethenet_header_reply;
}

static void processARPReplyMessage(Node *node, Interface *iif, EthernetHeader *ethernet_header)
{
    std::cout << __FUNCTION__ << " : ARP reply msg recvd on interface " << iif->getName() << " of node " << iif->getNode()->getName() << std::endl;

    ARPTable *arp_table = const_cast<ARPTable *>(node->getARPTable());
    arp_table->updateFromARPReply((ARPHeader *)ethernet_header->payload, iif);
}

void sendARPBroadcastRequest(Node *node, Interface *oif, const std::string &ip_addr)
{
    EthernetHeader *ethenet_header = (EthernetHeader *)(new char[ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(ARPHeader)]);

    if (!oif) {
        oif = node->getMatchingSubnetInterface(IPAddress(ip_addr));
    }

    if (!oif) {
        std::cout << "Error : " << node->getName() << " : No eligible subnet for ARP resolution for IP address : " << ip_addr << std::endl;
        return;
    }

    /* STEP 1 : prepare ethernet header */
    ethenet_header->dst_mac = MACAddress::BROADCAST_MAC_ADDRESS;
    ethenet_header->src_mac = oif->getMACAddress();
    ethenet_header->type = ARP_MSG;

    /* STEP2 : prepare ARP Broadcast Request Msg out of oif */
    ARPHeader *arp_header = (ARPHeader *)ethenet_header->payload;
    arp_header->hw_type = 1;
    arp_header->proto_type = 0x0800;
    arp_header->hw_addr_len = sizeof(MACAddress);
    arp_header->proto_addr_len = 4;

    arp_header->op_code = ARP_BROAD_REQ;

    arp_header->src_mac = oif->getMACAddress();
    arp_header->src_ip = oif->getIPAddress();

    arp_header->dst_mac = MACAddress();
    arp_header->dst_ip = IPAddress(ip_addr);

    /* DO NOT use ethernet_header->FCS = 0, because FCS lies at the
       end of payload, and not at the end of ethernet header!! */
    ETH_FCS(ethenet_header, sizeof(ARPHeader)) = 0; // unused

    /* STEP 3 : Now dispatch the ARP Broadcast Request Packet out of interface */
    oif->sendPacketOut(reinterpret_cast<char *>(ethenet_header), ETH_HDR_SIZE_EXCL_PAYLOAD + sizeof(ARPHeader));

    delete[] ethenet_header;
}

static void processARPBroadcastRequest(Node *node, Interface *iif, EthernetHeader *ethernet_header)
{
    std::cout << __FUNCTION__ << " : ARP Broadcast msg recvd on interface " << iif->getName() << " of node " << node->getName() << std::endl;
    ARPHeader *arp_header = reinterpret_cast<ARPHeader *>(ethernet_header->payload);
    IPAddress arp_dst_ip = IPAddress(arp_header->dst_ip);

    if (iif->getIPAddress() != arp_dst_ip) {
        std::cout <<
            node->getName() <<
            " : ARP Broadcast request message dropped, dst IP address " <<
            static_cast<std::string>(arp_dst_ip) <<
            " did not match with interface IP : " <<
            static_cast<std::string>(iif->getIPAddress()) <<
            std::endl;
        return;
    }
    sendARPReplyMessage(ethernet_header, iif);
}

/* VLAN APIs */
VLANEthernetHeader *tagPacketWithVLANID(EthernetHeader *ethernet_header, uint32_t total_packet_size, int32_t vlan_id, uint32_t *new_packet_size)
{
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); p) {
        VLANEthernetHeader *vlan_ethernet_header = reinterpret_cast<VLANEthernetHeader *>(ethernet_header);
        vlan_ethernet_header->vlan_8021q_header.tci_vid = vlan_id;
        *new_packet_size = total_packet_size;
        return vlan_ethernet_header;
    }
    char *head_position = reinterpret_cast<char *>(ethernet_header) - sizeof(VLAN8021QHeader);
    char *iterator = head_position;
    // dst_mac
    memmove(iterator, &ethernet_header->dst_mac, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    // src_mac
    memmove(iterator, &ethernet_header->src_mac, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    VLAN8021QHeader *vlan_8021q_header = reinterpret_cast<VLAN8021QHeader *>(iterator);
    vlan_8021q_header->tpid = 0x8100;
    vlan_8021q_header->tci_dei = 0;
    vlan_8021q_header->tci_pcp = 0;
    vlan_8021q_header->tci_vid = vlan_id;
    *new_packet_size = total_packet_size + sizeof(VLAN8021QHeader);
    return reinterpret_cast<VLANEthernetHeader *>(head_position);
}

EthernetHeader *untagPacketWithVLANID(EthernetHeader *ethernet_header, uint32_t total_packet_size, uint32_t *new_packet_size)
{
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); !p) {
        *new_packet_size = total_packet_size;
        return ethernet_header;
    }
    EthernetHeader *new_ethernet_header = reinterpret_cast<EthernetHeader *>(reinterpret_cast<char *>(ethernet_header) + sizeof(VLAN8021QHeader));
    MACAddress dst_mac = ethernet_header->dst_mac;
    MACAddress src_mac = ethernet_header->src_mac;
    new_ethernet_header->dst_mac = dst_mac;
    new_ethernet_header->src_mac = src_mac;
    *new_packet_size = total_packet_size - sizeof(VLAN8021QHeader);
    return new_ethernet_header;
}
