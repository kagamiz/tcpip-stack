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

    virtual void dump() const override;
private:
    const L3Route *lookup(const IPAddress &ip_addr, char mask) const;
    void deleteEntry(const IPAddress &ip_addr, char mask);
    std::list<L3Route> routing_table;
};

RoutingTable *getNewRoutingTable();
void deleteRoutingTable(RoutingTable *routing_table);
