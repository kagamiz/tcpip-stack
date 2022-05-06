/**
 * @file net.hpp
 * @author Jayson Sho Toma
 * @brief stores data structures and API for configuring network
 * @version 0.1
 * @date 2022-05-03
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "printer.hpp"

 // forward declaration
class ARPTable;
class MACTable;

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

    bool operator!=(const IPAddress &rhs) const
    {
        return !(*this == rhs);
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

    uint64_t getBitRepresentation() const
    {
        uint64_t result = 0;
        for (const auto &byte : mac) {
            result = (result << 8) | byte;
        }
        return result;
    }

    /**
     * @brief compares whether two MAC addresses are same
     *
     * @param rhs MAC address to be compared
     * @return true if bit representation of the MAC address are equal
     * @return false otherwise
     */
    bool operator==(const MACAddress &rhs) const
    {
        return getBitRepresentation() == rhs.getBitRepresentation();
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

    ~NodeNetworkProperty();

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

    const ARPTable *getARPTable() const
    {
        return arp_table;
    }

    const MACTable *getMACTable() const
    {
        return mac_table;
    }

    /**
     * @brief outputs a detail of this node property on the standard output
     *
     */
    virtual void dump() const override;

private:

    /* L2 properties */
    ARPTable *arp_table;
    MACTable *mac_table;

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

    enum class L2Mode {
        ACCESS,
        TRUNK,
        L2_MODE_UNKOWN,
    };

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
        l2mode = L2Mode::L2_MODE_UNKOWN;
        resetVLANSetting();
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

    const L2Mode &getL2Mode() const
    {
        return l2mode;
    }

    const std::string &getL2ModeStr() const
    {
        return L2ModeStr[static_cast<int>(l2mode)];
    }

    void setL2Mode(const L2Mode &mode)
    {
        l2mode = mode;
    }

    void resetVLANSetting();

    bool isVLANMember(uint32_t vlan_id) const;

    bool isVLANOccupied() const;

    void updateVLANMemberShips(uint32_t vlan_id);

    void addVLANMemberships(uint32_t vlan_id);

    const uint32_t getVLANID() const;

    /**
     * @brief outputs a detail of this interface property on the standard output
     *
     */
    virtual void dump() const override;

private:
    inline static const std::string L2ModeStr[] = {
        "access",
        "trunk",
        "L2_MODE_UNKNOWN",
    };

    /* L2 properties */
    inline static constexpr uint32_t MAX_VLAN_MEMBERSHIP = 10;
    MACAddress mac_addr; // hard burnt in interface NIC
    L2Mode l2mode;
    std::array<uint16_t, MAX_VLAN_MEMBERSHIP> vlans;

    /* L3 properties */
    bool is_ip_addr_configured; /* set to true if IP address is configured
                                   interface operates in L3 mode if IP address is configured on it */
    IPAddress ip_addr;
    char mask;
};

char *packetBufferShiftRight(char *packet, uint32_t packet_size, uint32_t total_buffer_size);
