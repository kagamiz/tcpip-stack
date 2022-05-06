/**
 * @file layer3.cpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-06
 */

#include "layer3.hpp"

#include <algorithm>
#include <iostream>

#include "../color.hpp"

IPHeader::IPHeader() :
    version(4),
    ihl(5),
    tos(0),
    total_length(0), // to be filled by the caller
    // fragmentation related field is currently not used in our implementation
    identification(0),
    unused_flag(0),
    DF_flag(1),
    MORE_flag(0),
    frag_offset(0),
    ttl(64),
    protocol(0), // to be filled by the caller
    checksum(0), // currently not used
    src_ip(0), // to be filled by the caller
    dst_ip(0) // to be filled by the caller
{

}

L3Route::L3Route() :
    dest(0),
    mask(0),
    is_direct(false),
    gateway_ip(0),
    oif("")
{

}

L3Route::L3Route(const IPAddress &_dest, char _mask, bool _is_direct, const IPAddress &_gateway_ip, const std::string &_oif) :
    dest(_dest),
    mask(_mask),
    is_direct(_is_direct),
    gateway_ip(_gateway_ip),
    oif(_oif)
{

}

void RoutingTable::addDirectRoute(const IPAddress &dst, char mask)
{
    addRoute(dst, mask, "", "");
}

void RoutingTable::addRoute(const IPAddress &dst, char mask, const std::string &gateway, const std::string &oif)
{
    IPAddress dst_subnet = dst.applyMask(mask);
    L3Route route(dst_subnet, mask, gateway.empty(), IPAddress(gateway), oif);

    if (const L3Route *old_route = lookup(dst_subnet, mask); old_route) {
        deleteEntry(old_route->dest, old_route->mask);
    }

    routing_table.push_back(route);
}

const L3Route *RoutingTable::lookupLPM(const IPAddress &dest_ip) const
{
    const L3Route *result = nullptr;
    for (const auto &route : routing_table) {
        if (route.dest.applyMask(route.mask) != dest_ip.applyMask(route.mask)) {
            continue;
        }
        if (!result || result->mask < route.mask) {
            result = &route;
        }
    }
    return result;
}

const L3Route *RoutingTable::lookup(const IPAddress &ip_addr, char mask) const
{
    auto result = std::find_if(
        std::begin(routing_table),
        std::end(routing_table),
        [&](const L3Route &route) {
            return route.dest == ip_addr && route.mask == mask;
        }
    );
    if (result == std::end(routing_table)) {
        return nullptr;
    }
    return &(*result);
}

void RoutingTable::dump() const
{
    std::cout << "   L3 Routing Table :" << std::endl;
    for (const auto &route : routing_table) {
        std::cout
            << "    * "
            << getColoredString(static_cast<std::string>(route.dest), "Light Red") << "/" << static_cast<int>(route.mask) << "\t"
            << " | "
            << (route.is_direct ? "NA" : getColoredString(static_cast<std::string>(route.gateway_ip), "Light Red")) << "\t"
            << " | "
            << (route.is_direct ? "NA" : route.oif) << "\t"
            << std::endl;
    }
}

void RoutingTable::deleteEntry(const IPAddress &ip_addr, char mask)
{
    routing_table.remove_if(
        [&](const L3Route &route) {
            return route.dest == ip_addr && route.mask == mask;
        }
    );
}

RoutingTable *getNewRoutingTable()
{
    return RoutingTable::getNewTable();
}

void deleteRoutingTable(RoutingTable *routing_table)
{
    delete routing_table;
}

void addDirectRouteEntryToRoutingTable(RoutingTable *routing_table, const std::string &ip_addr, char mask)
{
    routing_table->addDirectRoute(IPAddress(ip_addr), mask);
}