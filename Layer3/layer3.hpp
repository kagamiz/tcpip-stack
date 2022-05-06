/**
 * @file layer3.hpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-06
 */

#include <list>
#include <string>

#include "../net.hpp"
#include "../printer.hpp"

#pragma pack(push,1)
struct IPHeader {
    IPHeader();
    uint8_t version : 4;       /* version number, always 4 as we use IPv4 */
    uint8_t ihl : 4;           /* length of the IP header, in 32-bit words unit. always 5 on our implementation */
    uint8_t tos;               /* type of service */
    uint16_t total_length;     /* length of header + ip_header payload */

    // fragmentation related members. 
    // currently we won't perform fragmentation, so these members are intended to be not used.
    uint16_t identification;
    uint16_t unused_flag : 1;
    uint16_t DF_flag : 1;
    uint16_t MORE_flag : 1;
    uint16_t frag_offset : 13;

    uint8_t ttl;
    uint8_t protocol;
    uint16_t checksum;
    IPAddress src_ip;
    IPAddress dst_ip;
};
#pragma pack(pop)

#define IP_HDR_LEN_IN_BYTES(ip_hdr_ptr) (reinterpret_cast<IPHeader *>(ip_hdr_ptr)->ihl * 4)
#define IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) (reinterpret_cast<IPHeader *>(ip_hdr_ptr)->total_length)
#define INCREMENT_IPHDR(ip_hdr_ptr)(reinterpret_cast<char *>(ip_hdr_ptr) + IP_HDR_LEN_IN_BYTES(ip_hdr_ptr))
#define IP_HDR_PAYLOAD_SIZE(ip_hdr_ptr) (IP_HDR_TOTAL_LEN_IN_BYTES(ip_hdr_ptr) - IP_HDR_LEN_IN_BYTES(ip_hdr_ptr))

struct L3Route {
    L3Route();
    L3Route(const IPAddress &_dest, char _mask, bool _is_direct, const IPAddress &_gateway_ip, const std::string &_oif);

    bool operator==(const L3Route &rhs)
    {
        return dest == rhs.dest && mask == rhs.mask && is_direct == rhs.is_direct && gateway_ip == rhs.gateway_ip && oif == rhs.oif;
    }

    bool operator!=(const L3Route &rhs)
    {
        return !(*this == rhs);
    }

    IPAddress dest;
    char mask;
    bool is_direct;
    IPAddress gateway_ip;
    std::string oif;
};

class RoutingTable : public IPrinter {
public:
    RoutingTable() {}
    static RoutingTable *getNewTable()
    {
        return new RoutingTable();
    }

    void addDirectRoute(const IPAddress &dst, char mask);
    void addRoute(const IPAddress &dst, char mask, const std::string &gateway, const std::string &oif);
    const L3Route *lookupLPM(const IPAddress &dest_ip) const;
    void deleteEntry(const IPAddress &ip_addr, char mask);

    virtual void dump() const override;
private:
    const L3Route *lookup(const IPAddress &ip_addr, char mask) const;
    std::list<L3Route> routing_table;
};

RoutingTable *getNewRoutingTable();
void deleteRoutingTable(RoutingTable *routing_table);

void addDirectRouteEntryToRoutingTable(RoutingTable *routing_table, const std::string &ip_addr, char mask);
