/**
 * @file net.hpp
 * @author Jayson Sho Toma
 * @brief stores data structures and API for configuring network
 * @version 0.1
 * @date 2022-05-03
 */

#include <iomanip>
#include <iostream>
#include <sstream>

#include "color.hpp"
#include "net.hpp"

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
    is_loopback_addr_configured(false),
    loopback_addr("0.0.0.0")
{

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
    is_ip_addr_configured(false),
    ip_addr("0.0.0.0"),
    mask(0)
{

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
        << std::endl;
}
