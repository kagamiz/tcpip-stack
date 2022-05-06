/**
 * @file l2switch.cpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-05
 */

#include "l2switch.hpp"
#include "layer2.hpp"

#include <algorithm>
#include <iostream>

 /* L2 Switching APIs */
void nodeSetInterfaceL2Mode(Node *node, const std::string &interface_name, const InterfaceNetworkProperty::L2Mode &mode)
{
    Interface *intf = node->getNodeInterfaceByName(interface_name);
    if (!intf) {
        return;
    }
    intf->setL2Mode(mode);
}

void nodeSetInterfaceVLAN(Node *node, const std::string &interface_name, uint32_t vlan_id)
{
    Interface *intf = node->getNodeInterfaceByName(interface_name);
    if (!intf) {
        return;
    }
    intf->setVLANMemberships(vlan_id);
}

MACTableEntry::MACTableEntry() :
    mac_addr(),
    oif_name("")
{

}

MACTableEntry *MACTable::MACTableLookup(const MACAddress &mac_addr)
{
    auto result = std::find_if(
        std::begin(mac_table),
        std::end(mac_table),
        [&](const MACTableEntry &mac_table_entry)
        {
            return mac_table_entry.mac_addr == mac_addr;
        }
    );

    if (result == std::end(mac_table)) {
        return nullptr;
    }
    return &(*result);
}

void MACTable::deleteEntry(const MACAddress &mac_addr)
{
    mac_table.remove_if(
        [&](const MACTableEntry &mac_table_entry)
        {
            return mac_table_entry.mac_addr == mac_addr;
        });
}

bool MACTable::addEntry(MACTableEntry *mac_table_entry)
{
    MACTableEntry *mac_table_entry_old = MACTableLookup(mac_table_entry->mac_addr);
    // no need to add!
    if (mac_table_entry_old && memcmp(mac_table_entry_old, mac_table_entry, sizeof(MACTableEntry)) == 0) {
        // caller need to free MACTableEntry
        return false;
    }

    if (mac_table_entry_old) {
        deleteEntry(mac_table_entry_old->mac_addr);
    }

    mac_table.push_back(*mac_table_entry);

    return true;
}

void MACTable::dump() const
{
    for (const auto &mac_table_entry : mac_table) {
        std::cout <<
            "MAC : " <<
            static_cast<std::string>(mac_table_entry.mac_addr) <<
            " | Intf : " <<
            mac_table_entry.oif_name <<
            std::endl;
    }
}

MACTable *getNewMACTable()
{
    return MACTable::getNewTable();
}

void deleteMACTable(MACTable *mac_table)
{
    delete mac_table;
}

static void l2SwitchForwardFrame(Node *node, Interface *recv_intf, char *packet, uint32_t packet_size)
{
    EthernetHeader *ethernet_header = reinterpret_cast<EthernetHeader *>(packet);
    if (ethernet_header->dst_mac == MACAddress::BROADCAST_MAC_ADDRESS) {
        node->sendPacketFloodToL2Interface(recv_intf, packet, packet_size);
        return;
    }

    MACTable *mac_table = const_cast<MACTable *>(node->getMACTable());
    MACTableEntry *mac_table_entry = mac_table->MACTableLookup(ethernet_header->dst_mac);

    if (!mac_table_entry) {
        node->sendPacketFloodToL2Interface(recv_intf, packet, packet_size);
        return;
    }

    Interface *oif = node->getNodeInterfaceByName(mac_table_entry->oif_name);
    if (!oif) {
        return;
    }
    oif->sendPacketOut(packet, packet_size);
}

static void l2SwitchPerformMACLearning(Node *node, const MACAddress &src_mac, const std::string &if_name)
{
    MACTable *mac_table = const_cast<MACTable *>(node->getMACTable());
    MACTableEntry entry;
    entry.mac_addr = src_mac;
    entry.oif_name = if_name;
    mac_table->addEntry(&entry);
}

void l2SwitchRecvFrame(Interface *intf, char *packet, uint32_t packet_size)
{
    Node *node = const_cast<Node *>(intf->getNode());
    EthernetHeader *ethernet_header = reinterpret_cast<EthernetHeader *>(packet);

    l2SwitchPerformMACLearning(node, ethernet_header->src_mac, intf->getName());
    l2SwitchForwardFrame(node, intf, packet, packet_size);
}
