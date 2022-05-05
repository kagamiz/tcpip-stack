/**
 * @file l2switch.cpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-05
 */

#include "l2switch.hpp"

#include <algorithm>
#include <iostream>

 /* L2 Switching APIs */
void nodeSetInterfaceL2Mode(Node *node, const std::string &interface_name, const InterfaceNetworkProperty::L2Mode &mode)
{
    Interface *intf = node->getNodeInterfaceByName(interface_name);
    if (!intf) {
        return;
    }
    if (intf->isL3Mode()) {
        std::cout << "Error : interface " << interface_name << " is working as L3 Mode. Please unset the IP address to change the L2 mode." << std::endl;
        return;
    }
    intf->setL2Mode(mode);
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
            "| Intf : " <<
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
