/**
 * @file net.hpp
 * @author Jayson Sho Toma
 * @brief stores data structures and API for configuring network
 * @version 0.1
 * @date 2022-05-03
 */

#pragma once

#include <cstdint>
#include <string>
#include <cstring>

#include "printer.hpp"

 /**
  * @class IPAddress
  * @brief represents an IP address.
  *
  */
class IPAddress {
public:
    /**
     * @brief Construct a new IPAddress object
     *
     * @param addr 32-bit integer which represents an IP address
     */
    explicit IPAddress(uint32_t addr);
    /**
     * @brief Construct a new IPAddress object
     *
     * @param addr string of form ddd.ddd.ddd.ddd which represents an IP address
     */
    explicit IPAddress(const std::string &addr);

    /**
     * @brief cast operator which returns IP address as a 32-bit integer.
     *
     * @return uint32_t
     */
    operator uint32_t() const;
    /**
     * @brief cast operator which returns IP address as a string of the form ddd.ddd.ddd.ddd
     *
     * @return std::string
     */
    operator std::string() const;

    /**
     * @brief returns IP address masked with ~((1 << mask_size) - 1)
     *
     * @param mask_size
     * @return IPAddress
     */
    IPAddress applyMask(char mask_size) const;

    /**
     * @brief compares whether two IP addresses are same
     *
     * @param rhs IP address to be compared
     * @return true if 32-bit integer representation of the IP address are equal
     * @return false otherwise
     */
    bool operator==(const IPAddress &rhs) const
    {
        return static_cast<uint32_t>(*this) == static_cast<uint32_t>(rhs);
    }

private:
    uint32_t ip_addr;
};

/**
 * @class MACAddress
 * @brief represents a MAC address.
 *
 */
class MACAddress {
public:
    /**
     * @brief Construct a new MACAddress object. All the bits of the MAC address are initialized with zero.
     *
     */
    MACAddress();
    /**
     * @brief Construct a new MACAddress object.
     *        47th-bit to 0th-bit (counted from lsb) are recognized as a MAC address.
     *
     * @param val
     */
    explicit MACAddress(uint64_t val);

    /**
     * @brief cast operator which returns MAC address as a string of the form XX:XX:XX:XX:XX:XX.
     *
     * @return std::string
     */
    operator std::string() const;

    /**
     * @brief returns array which represents MAC address.
     *
     * @return const char*
     */
    const char *get() const
    {
        return mac;
    }

    /**
     * @brief compares whether two MAC addresses are same
     *
     * @param rhs MAC address to be compared
     * @return true if array representation of the MAC address are equal
     * @return false otherwise
     */
    bool operator==(const MACAddress &rhs) const
    {
        return std::memcmp(mac, rhs.get(), MAC_ADDRESS_BYTE_LENGTH) == 0;
    }

    /**
     * @brief broadcast MAC address (FF:FF:FF:FF:FF:FF).
     *
     */
    static const MACAddress BROADCAST_MAC_ADDRESS;

private:
    static constexpr uint8_t MAC_ADDRESS_BYTE_LENGTH = 6;
    char mac[MAC_ADDRESS_BYTE_LENGTH];
};

/**
 * @class NodeNetworkProperty
 * @brief stores network property used by network nodes.
 *
 */
class NodeNetworkProperty : public IPrinter {
public:
    /**
     * @brief Construct a new Node Network Property object
     *
     */
    NodeNetworkProperty();

    /**
     * @brief Get the Loopback Address object
     *
     * @return const IPAddress&
     */
    const IPAddress &getLoopbackAddress() const
    {
        return loopback_addr;
    }

    /**
     * @brief Set the Loopback Address object
     *
     * @param addr loopback IP address.
     */
    void setLoopbackAddress(const IPAddress &addr)
    {
        loopback_addr = addr;
        is_loopback_addr_configured = true;
    }

    /**
     * @brief outputs a detail of this node property on the standard output
     *
     */
    virtual void dump() const override;

private:
    /* L3 properties */
    bool is_loopback_addr_configured;
    IPAddress loopback_addr;
};

/**
 * @class InterfaceNetworkProperty
 * @brief stores network property used by interfaces.
 *
 */
class InterfaceNetworkProperty : public IPrinter {
public:
    /**
     * @brief Construct a new Interface Network Property object
     *
     */
    InterfaceNetworkProperty();

    /**
     * @brief returns MAC address associated to the interface
     *
     * @return const MACAddress&
     */
    const MACAddress &getMACAddress() const
    {
        return mac_addr;
    }

    /**
     * @brief sets MAC address for the interface
     *
     * @param addr MAC address
     */
    void setMACAddress(const MACAddress &addr)
    {
        mac_addr = addr;
    }

    /**
     * @brief get IP address associated to the interface
     *
     * @return const IPAddress&
     */
    const IPAddress &getIPAddress() const
    {
        return ip_addr;
    }

    /**
     * @brief Get bit length of the subnet mask
     *
     * @return char
     */
    char getMask() const
    {
        return mask;
    }

    /**
     * @brief sets IP address and subnet mask for the interface
     *
     * @param addr IP address
     * @param subnet_mask bit length of the subnet mask
     */
    void setIPAddress(const IPAddress &addr, const char &subnet_mask)
    {
        ip_addr = addr;
        mask = subnet_mask;
        is_ip_addr_configured = true;
    }

    /**
     * @brief unsets IP address associated to the interface
     *
     */
    void unsetIPAddress()
    {
        is_ip_addr_configured = false;
        // TODO: not clearing `ip_addr` and `mask` might cause a security incident
    }

    /**
     * @brief checks wheter this interface is acting as a L3 component.
     *
     * @return true if IP address is configured.
     * @return false otherwise.
     */
    bool isL3Mode() const
    {
        return is_ip_addr_configured;
    }

    /**
     * @brief outputs a detail of this interface property on the standard output
     *
     */
    virtual void dump() const override;

private:
    /* L2 properties */
    MACAddress mac_addr; // hard burnt in interface NIC

    /* L3 properties */
    bool is_ip_addr_configured; /* set to true if IP address is configured
                                   interface operates in L3 mode if IP address is configured on it */
    IPAddress ip_addr;
    char mask;
};
