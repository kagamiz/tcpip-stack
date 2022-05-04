/**
 * @file layer2.cpp
 * @author Jayson Sho Toma
 * @brief defines layer 2 related implementation
 * @version 0.1
 * @date 2022-05-04
 */

#include "../color.hpp"
#include "../tcpconst.hpp"
#include "layer2.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>

void layer2FrameRecv(Node *node, Interface *interface, char *packet, uint32_t packet_size)
{
    /* Entry point into TCP/IP from bottom */
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

void sendARPBroadcastRequest(Node *node, Interface *oif, const std::string &ip_addr)
{
    uint32_t payload_size = sizeof(ARPHeader);
    EthernetHeader *ethenet_header = (EthernetHeader *)calloc(sizeof(char), ETH_HDR_SIZE_EXCL_PAYLOAD + payload_size);

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
    oif->sendPacketOut(reinterpret_cast<char *>(ethenet_header), sizeof(EthernetHeader));
}
