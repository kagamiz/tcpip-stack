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

#include "printer.hpp"

class IPAddress {
public:
    explicit IPAddress(uint32_t addr);
    explicit IPAddress(const std::string &addr);

    operator uint32_t() const;
    operator std::string() const;

private:
    uint32_t ip_addr;
};

class MACAddress {
public:
    MACAddress();
    explicit MACAddress(uint64_t val);

    operator std::string() const;
private:
    static constexpr uint8_t MAC_ADDRESS_BYTE_LENGTH = 6;
    char mac[MAC_ADDRESS_BYTE_LENGTH];
};

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
     * @param addr
     */
    void setLoopbackAddress(const IPAddress &addr)
    {
        loopback_addr = addr;
        is_loopback_addr_configured = true;
    }

    virtual void dump() const override;

private:
    /* L3 properties */
    bool is_loopback_addr_configured;
    IPAddress loopback_addr;
};

class InterfaceNetworkProperty : public IPrinter {
public:
    /**
     * @brief Construct a new Interface Network Property object
     *
     */
    InterfaceNetworkProperty();

    /**
     * @brief
     *
     * @return const MACAddress&
     */
    const MACAddress &getMACAddress() const
    {
        return mac_addr;
    }

    /**
     * @brief
     *
     * @param addr
     */
    void setMACAddress(const MACAddress &addr)
    {
        mac_addr = addr;
    }

    /**
     * @brief
     *
     * @return const IPAddress&
     */
    const IPAddress &getIPAddress() const
    {
        return ip_addr;
    }

    /**
     * @brief
     *
     * @param addr
     */
    void setIPAddress(const IPAddress &addr, const char &subnet_mask)
    {
        ip_addr = addr;
        mask = subnet_mask;
        is_ip_addr_configured = true;
    }

    /**
     * @brief
     *
     */
    void unsetIPAddress()
    {
        is_ip_addr_configured = false;
        // TODO: not clearing `ip_addr` and `mask` might cause a security incident
    }

    bool isL3Mode() const
    {
        return is_ip_addr_configured;
    }

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
