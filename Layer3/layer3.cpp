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
#include "../comm.hpp"
#include "../tcpconst.hpp"

extern void demotePacketToLayer2(Node *node, const IPAddress &nexthop_ip, const Interface *oif, char *packet, uint32_t packet_size, int protocol_number);

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
        if (route == *old_route) {
            return;
        }
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

void deleteEntryFromRoutingTable(RoutingTable *routing_table, const std::string &ip_addr, char mask)
{
    routing_table->deleteEntry(IPAddress(ip_addr), mask);
}

bool isLayer3LocalDelivery(Node *node, const IPAddress &dest_ip)
{
    if (node->isLoopbackAddressConfigured() && node->getLoopbackAddress() == dest_ip) {
        return true;
    }

    return node->getNodeInterfaceByIPAddress(dest_ip);
}

static void layer3IPPacketRecvFromLayer2(Node *node, Interface *interface, IPHeader *ip_header, uint32_t packet_size)
{
    IPAddress dest_ip = ip_header->dst_ip;
    const L3Route *l3_route = node->getRoutingTable()->lookupLPM(dest_ip);

    if (!l3_route) {
        std::cout << "Router " << node->getName() << " : Cannot Route IP : " << getColoredString(static_cast<std::string>(dest_ip), "Light Red") << std::endl;
        return;
    }

    // router accepts the IP header

    if (l3_route->is_direct) {
        if (isLayer3LocalDelivery(node, dest_ip)) {
            // local delivery case
            switch (ip_header->protocol) {
            case ICMP_PRO:
                std::cout << "IP Address : " << getColoredString(static_cast<std::string>(dest_ip), "Light Red") << ", ping success" << std::endl;
                break;
            default:
                break;
            }
        }
        else {
            // direct host delivery case
            demotePacketToLayer2(node, IPAddress(0), nullptr, reinterpret_cast<char *>(ip_header), packet_size, ETH_IP);
        }
    }
    else {
        // L3 forwarding case
        ip_header->ttl--;

        // drop the packet
        if (ip_header->ttl == 0) {
            return;
        }

        Interface *oif = node->getNodeInterfaceByName(l3_route->oif);
        demotePacketToLayer2(
            node,
            l3_route->gateway_ip,
            oif,
            reinterpret_cast<char *>(ip_header),
            packet_size,
            ETH_IP
        );
    }
}

static void layer3PacketRecvFromLayer2(Node *node, Interface *interface, char *packet, uint32_t packet_size, int l3_protocol_number)
{
    switch (l3_protocol_number) {
    case ETH_IP:
    {
        layer3IPPacketRecvFromLayer2(node, interface, reinterpret_cast<IPHeader *>(packet), packet_size);
        break;
    }
    // TBD
    // case ETH_IPv6:
    // ...
    //
    default:
        break;
    }
}

/**
 * @brief A public API to be used by L2 or othe lower layers to promote packets to layer3 in the TCP/IP stack
 *
 * @param node
 * @param recv_intf
 * @param payload
 * @param app_data_size
 * @param l3_protocol_number // obtained from "type" field of the EthernetHeader
 */
void promotePacketToLayer3(Node *node, Interface *recv_intf, char *payload, uint32_t app_data_size, int l3_protocol_number)
{
    layer3PacketRecvFromLayer2(node, recv_intf, payload, app_data_size, l3_protocol_number);
}

static void layer3RecvPacketFromTop(Node *node, char *packet, uint32_t packet_size, int protocol_number, const IPAddress &dest_ip_address)
{
    IPHeader ip_header;
    ip_header.protocol = protocol_number;
    ip_header.src_ip = node->getLoopbackAddress();
    ip_header.dst_ip = dest_ip_address;
    ip_header.total_length = ip_header.ihl * 4 + packet_size;

    const L3Route *l3_route = node->getRoutingTable()->lookupLPM(dest_ip_address);
    if (!l3_route) {
        std::cout << "Node " << node->getName() << " : No L3 route for IP address " << getColoredString(static_cast<std::string>(dest_ip_address), "Light Red") << std::endl;
        return;
    }

    char *new_packet = new char[MAX_PACKET_BUFFER_SIZE];
    memcpy(new_packet, reinterpret_cast<char *>(&ip_header), ip_header.ihl * 4);
    if (packet && packet_size) {
        memcpy(new_packet + ip_header.ihl * 4, packet, packet_size);
    }

    IPAddress next_hop_ip = (l3_route->is_direct ? dest_ip_address : l3_route->gateway_ip);
    uint32_t new_packet_size = packet_size + ip_header.ihl * 4;
    char *shifted_packet_buffer = packetBufferShiftRight(new_packet, packet_size + new_packet_size, MAX_PACKET_BUFFER_SIZE);

    Interface *oif = (l3_route->is_direct ? nullptr : node->getNodeInterfaceByName(l3_route->oif));
    demotePacketToLayer2(node, next_hop_ip, oif, shifted_packet_buffer, new_packet_size, ETH_IP);

    delete[] new_packet;
}

/**
 * @brief An API to be used by L4 or L5 to push the packet down to the TCP/IP stack L3.
 *
 * @param node
 * @param packet
 * @param packet_size
 * @param protocol_number   // L4 or L5 protocol type
 * @param dest_ip_address
 */
void demotePacketToLayer3(Node *node, char *packet, uint32_t packet_size, int protocol_number, const IPAddress &dest_ip_address)
{
    layer3RecvPacketFromTop(node, packet, packet_size, protocol_number, dest_ip_address);
}
