/**
 * @file net.cpp
 * @author Jayson Sho Toma
 * @brief stores data structures and API for configuring network
 * @version 0.1
 * @date 2022-05-03
 */

#include <algorithm>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "color.hpp"
#include "net.hpp"

extern ARPTable *getNewARPTable();
extern void deleteARPTable(ARPTable *arp_table);

extern MACTable *getNewMACTable();
extern void deleteMACTable(MACTable *mac_table);

extern RoutingTable *getNewRoutingTable();
extern void deleteRoutingTable(RoutingTable *routing_table);

MACAddress::MACAddress() :
    mac{ 0 }
{

}

MACAddress::MACAddress(uint64_t val)
{
    for (int i = MAC_ADDRESS_BYTE_LENGTH - 1; i >= 0; i--) {
        mac[i] = val & 0xFF;
        val >>= 8;
    }
}

MACAddress::operator std::string() const
{
    std::string result;
    for (int i = 0; i < MAC_ADDRESS_BYTE_LENGTH; i++) {
        std::stringstream ss;
        ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (int)(mac[i] & 0xFF);
        result = result + (result.empty() ? "" : ":") + ss.str();
    }
    return result;
}

const MACAddress MACAddress::BROADCAST_MAC_ADDRESS = MACAddress(0xFFFFFFFFFFFFll);

IPAddress::IPAddress(uint32_t addr) :
    ip_addr(addr)
{
}

IPAddress::IPAddress(const std::string &addr)
{
    std::istringstream iss(addr);
    std::string token;
    uint32_t result = 0;
    while (std::getline(iss, token, '.')) {
        result = (result << 8) | std::stoi(token);
    }
    ip_addr = result;
}

IPAddress::operator uint32_t() const
{
    return ip_addr;
}

IPAddress::operator std::string() const
{
    std::string result;
    uint32_t ip_addr_tmp = ip_addr;
    for (int i = 0; i < 4; i++) {
        std::string delimiter = (result.empty() ? "" : ".");
        result = std::to_string(ip_addr_tmp & 0xFF) + delimiter + result;
        ip_addr_tmp >>= 8;
    }
    return result;
}

IPAddress IPAddress::applyMask(char mask_size) const
{
    return IPAddress(ip_addr & ~((1ll << (32 - mask_size)) - 1));
}

NodeNetworkProperty::NodeNetworkProperty() :
    arp_table(getNewARPTable()),
    mac_table(getNewMACTable()),
    routing_table(getNewRoutingTable()),
    is_loopback_addr_configured(false),
    loopback_addr("0.0.0.0")
{
}

NodeNetworkProperty::~NodeNetworkProperty()
{
    if (arp_table) {
        deleteARPTable(arp_table);
    }
    arp_table = nullptr;

    if (mac_table) {
        deleteMACTable(mac_table);
    }
    mac_table = nullptr;

    if (routing_table) {
        deleteRoutingTable(routing_table);
    }
    routing_table = nullptr;
}

void NodeNetworkProperty::dump() const
{
    std::cout
        << "  "
        << "lo addr : "
        << (is_loopback_addr_configured ? getColoredString(static_cast<std::string>(loopback_addr), "Light Red") + "/32" : getColoredString("unset", "Light Gray"))
        << std::endl;
}

InterfaceNetworkProperty::InterfaceNetworkProperty() :
    mac_addr(),
    l2mode(L2Mode::L2_MODE_UNKOWN),
    is_ip_addr_configured(false),
    ip_addr("0.0.0.0"),
    mask(0)
{
    std::fill(std::begin(vlans), std::end(vlans), 0);
}

void InterfaceNetworkProperty::resetVLANSetting()
{
    std::fill(std::begin(vlans), std::end(vlans), 0);
}

bool InterfaceNetworkProperty::isVLANMember(uint32_t vlan_id) const
{
    // 0 is invalid for VLAN ID.
    if (vlan_id == 0) {
        return false;
    }

    auto result = std::find_if(
        std::begin(vlans),
        std::end(vlans),
        [&](const uint32_t &vlan_member_id) {
            return vlan_id == vlan_member_id;
        }
    );
    return result != std::end(vlans);
}

bool InterfaceNetworkProperty::isVLANOccupied() const
{
    auto result = std::find_if(
        std::begin(vlans),
        std::end(vlans),
        [](const uint32_t &vlan_member_id) {
            return vlan_member_id == 0;
        }
    );
    return result != std::end(vlans);
}

void InterfaceNetworkProperty::updateVLANMemberShips(uint32_t vlan_id)
{
    *(std::begin(vlans)) = vlan_id;
}

void InterfaceNetworkProperty::addVLANMemberships(uint32_t vlan_id)
{
    if (isVLANMember(vlan_id)) {
        return;
    }

    for (auto &vlan_member : vlans) {
        if (vlan_member != 0) {
            continue;
        }
        vlan_member = vlan_id;
        return;
    }

    std::cout << "Error : cannot enroll id " << vlan_id << " as a VLAN member : memberships already occupied" << std::endl;
    return;
}

const uint32_t InterfaceNetworkProperty::getVLANID() const
{
    if (l2mode != InterfaceNetworkProperty::L2Mode::ACCESS) {
        std::cout << "Error : " << __FUNCTION__ << " is not supposed to be called on L2 Mode " << getL2ModeStr() << std::endl;
        assert(false);
    }
    return vlans.front();
}

void InterfaceNetworkProperty::dump() const
{
    std::cout
        << "  "
        << "IP addr : "
        << (is_ip_addr_configured ? getColoredString(static_cast<std::string>(ip_addr), "Light Red") + "/" + std::to_string(mask) : getColoredString("unset", "Light Gray"))
        << "  "
        << "MAC : "
        << static_cast<std::string>(mac_addr)
        << "  "
        << "L2 Mode : "
        << getL2ModeStr()
        << std::endl;

    if (l2mode == L2Mode::ACCESS || l2mode == L2Mode::TRUNK) {
        std::cout << "  VLAN ID(s) :" << std::endl;
        for (const auto &vlan_id : vlans) {
            if (vlan_id == 0) {
                continue;
            }
            std::cout << "   * " << vlan_id << std::endl;
        }
    }
}

char *packetBufferShiftRight(char *packet, uint32_t packet_size, uint32_t total_buffer_size)
{
    uint32_t head_position = total_buffer_size - packet_size;
    memmove(packet + head_position, packet, packet_size);
    return packet + head_position;
}
