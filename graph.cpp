/**
 * @file graph.cpp
 * @author Jayson Sho Toma
 * @brief graph data structure which is used to represent computer networks.
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <thread>

Interface::Interface(const std::string &name) :
    if_name(name.substr(0, MAX_INTF_NAME_LENGTH)),
    att_node(nullptr),
    link(nullptr)
{
}

const Node *Interface::getNeighbourNode() const
{
    // error handling
    if (!att_node) {
        return nullptr;
    }
    if (!link) {
        return nullptr;
    }

    // regular cases
    if (link->getFromInterface()->getNode() == att_node) {
        return link->getToInterface()->getNode();
    }
    if (link->getToInterface()->getNode() == att_node) {
        return link->getFromInterface()->getNode();
    }

    // irregular case
    return nullptr;
}

void Interface::assignMACAddress()
{
    auto calcHashCode = [](const std::string &s) -> uint64_t {
        uint64_t result = 0;
        for (const auto &c : s) {
            result = result * 97 + c;
        }
        return result;
    };

    uint64_t hash = calcHashCode(if_name) * calcHashCode(att_node->getName());
    std::mt19937 rand_src(hash);
    std::uniform_int_distribution<uint64_t> rand_dist(0, (1ull << 46) - 1);
    intf_network_property.setMACAddress(MACAddress(rand_dist(rand_src)));
}

void Interface::setIPAddress(const std::string &ip_addr, char mask)
{
    intf_network_property.setIPAddress(IPAddress(ip_addr), mask);
}

void Interface::unsetIPAddress()
{
    intf_network_property.unsetIPAddress();
}

bool Interface::isL3Mode() const
{
    return intf_network_property.isL3Mode();
}

void Interface::dump() const
{
    std::cout
        << " Local Node : "
        << (getNode() ? getNode()->getName() : "(undefined)") << ", "
        << "Interface Name : " << if_name << ", "
        << "Nbr Node : "
        << (getNeighbourNode() ? getNeighbourNode()->getName() : "(undefined)") << ", "
        << "cost : ";

    if (getLink()) {
        std::cout << getLink()->getCost() << std::endl;
    }
    else {
        std::cout << "(undefined)" << std::endl;
    }
    intf_network_property.dump();
}

Node::Node(const std::string &name) :
    node_name(name.substr(0, MAX_NODE_NAME_LENGTH)),
    udp_port_number(0),
    udp_sock_fd(-1)
{
    std::fill(std::begin(intfs), std::end(intfs), nullptr);
    initUDPSocket();
}

Node::~Node()
{
    for (auto &intf : intfs) {
        if (!intf) {
            continue;
        }
        Link *link = const_cast<Link *>(intf->getLink());
        delete link;
        link = nullptr;
    }
}

// legacy function
int32_t Node::getNodeInterfaceAvailableSlot()
{
    auto result = std::find_if(std::begin(intfs), std::end(intfs), [](Interface *p) -> bool {return !p;});
    if (result == std::end(intfs)) {
        return -1;
    }
    else {
        return static_cast<int32_t>(result - std::begin(intfs));
    }
}

bool Node::hasVacantInterfaceSlot() const
{
    auto result = std::find_if(std::begin(intfs), std::end(intfs), [](Interface *p) -> bool {return !p;});
    return result != std::end(intfs);
}

bool Node::trySetInterfaceToSlot(Interface *intf)
{
    for (auto &intf_ref : intfs) {
        if (intf_ref) {
            continue;
        }
        intf_ref = intf;
        return true;
    }
    return false;
}

bool Node::tryRemoveInterfaceFromSlot(Interface *intf)
{
    for (auto &intf_ref : intfs) {
        if (intf_ref != intf) {
            continue;
        }
        intf_ref = nullptr;
        return true;
    }
    return false;
}

Interface *Node::getNodeInterfaceByName(const std::string &if_name)
{
    auto result = std::find_if(
        std::begin(intfs),
        std::end(intfs),
        [&](Interface *intf) {
            if (!intf) {
                return false;
            }
            return intf->getName() == if_name;
        }
    );
    if (result == std::end(intfs)) {
        return nullptr;
    }
    return *result;
}

Interface *Node::getMatchingSubnetInterface(const std::string &ip_addr)
{
    IPAddress input_ip(ip_addr);
    auto result = std::find_if(
        std::begin(intfs),
        std::end(intfs),
        [&](Interface *intf) {
            if (!intf) {
                return false;
            }
            if (!intf->isL3Mode()) {
                return false;
            }
            char mask_size = intf->getMask();
            return intf->getIPAddress().applyMask(mask_size) == input_ip.applyMask(mask_size);
        }
    );
    if (result == std::end(intfs)) {
        return nullptr;
    }
    return *result;
}

bool Node::setLoopbackAddress(const std::string &ip_addr)
{
    node_network_property.setLoopbackAddress(IPAddress(ip_addr));
    return true;
}

bool Node::setInterfaceIPAddress(const std::string &if_name, const std::string &ip_addr, char mask)
{
    Interface *intf = getNodeInterfaceByName(if_name);
    if (!intf) {
        return false;
    }
    intf->setIPAddress(ip_addr, mask);
    return true;
}

bool Node::unsetInterfaceIPAddress(const std::string &if_name)
{
    Interface *intf = getNodeInterfaceByName(if_name);
    if (!intf) {
        return false;
    }
    intf->unsetIPAddress();
    return true;
}

void Node::receivePacket(char *packet_with_aux_data, uint32_t packet_size)
{
    // TBD
}

void Node::dump() const
{
    std::cout << "Node Name = " << node_name << ":" << std::endl;
    node_network_property.dump();
    for (const auto &intf : intfs) {
        if (!intf) {
            continue;
        }
        intf->dump();
    }
}

uint32_t Node::generateUDPPortNumber()
{
    return memoized_udp_port_number++;
}

void Node::initUDPSocket()
{
    udp_port_number = generateUDPPortNumber();
    udp_sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (udp_sock_fd < 0) {
        std::cout << "Error : socket() failed for Node " << node_name << std::endl;
        return;
    }

    sockaddr_in node_addr;
    node_addr.sin_family = AF_INET;
    node_addr.sin_port = udp_port_number;
    node_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_sock_fd, reinterpret_cast<sockaddr *>(&node_addr), sizeof(sockaddr)) < 0) {
        std::cout << "Error : socket bind failed for Node " << node_name << std::endl;
        return;
    }
}

Link::Link(const std::string &from_if_name, const std::string &to_if_name, uint32_t _cost) :
    intf1(from_if_name),
    intf2(to_if_name),
    cost(_cost)
{
}

Link::~Link()
{
    Node *node1 = const_cast<Node *>(intf1.getNode());
    node1->tryRemoveInterfaceFromSlot(&intf1);

    Node *node2 = const_cast<Node *>(intf2.getNode());
    node2->tryRemoveInterfaceFromSlot(&intf2);
}

Link *Link::tryCreate(Node *node1, Node *node2, const std::string &from_if_name, const std::string &to_if_name, uint32_t cost)
{
    if (!node1->hasVacantInterfaceSlot() || !node2->hasVacantInterfaceSlot()) {
        return nullptr;
    }
    Link *link = new Link(from_if_name, to_if_name, cost);

    node1->trySetInterfaceToSlot(link->getFromInterface());
    link->getFromInterface()->setNode(node1);
    link->getFromInterface()->setLink(link);
    link->getFromInterface()->assignMACAddress();

    node2->trySetInterfaceToSlot(link->getToInterface());
    link->getToInterface()->setNode(node2);
    link->getToInterface()->setLink(link);
    link->getToInterface()->assignMACAddress();

    return link;
}

Graph::Graph(const std::string &name) :
    topology_name(name.substr(0, MAX_TOPOLOGY_NAME_LENGTH))
{
}

Graph::~Graph()
{
    for (auto &node : nodes) {
        delete node;
        node = nullptr;
    }
}

Node *Graph::addNode(const std::string &node_name)
{
    Node *node = new Node(node_name);
    if (!node) {
        return nullptr;
    }
    nodes.push_back(node);
    return node;
}

bool Graph::insertLinkBetweenTwoNodes(Node *node1, Node *node2, const std::string &from_if_name, const std::string &to_if_name, uint32_t cost)
{
    if (std::find(std::begin(nodes), std::end(nodes), node1) == std::end(nodes)) {
        return false;
    }
    if (std::find(std::begin(nodes), std::end(nodes), node2) == std::end(nodes)) {
        return false;
    }

    Link *link = Link::tryCreate(node1, node2, from_if_name, to_if_name, cost);
    if (!link) {
        return false;
    }

    return true;
}

Node *Graph::getNodeByNodeName(const std::string &node_name)
{
    auto result = std::find_if(std::begin(nodes), std::end(nodes), [&](Node *node) -> bool { return node->getName() == node_name;});
    if (result == std::end(nodes)) {
        return nullptr;
    }
    return *result;
}

void Graph::startPacketReceiverThread()
{
    std::thread t
    ([this] {
        fd_set active_sock_fd_set, backup_sock_fd_set;

        int sock_max_fd = 0;
        int bytes_recvd = 0;

        int addr_len = sizeof(sockaddr);

        FD_ZERO(&active_sock_fd_set);
        FD_ZERO(&backup_sock_fd_set);

        sockaddr_in sender_addr;

        for (const auto &node : nodes) {
            int sock_fd = node->getUDPSocketFileDescriptor();
            if (sock_fd < 0) {
                continue;
            }
            FD_SET(sock_fd, &backup_sock_fd_set);
            sock_max_fd = std::max(sock_max_fd, sock_fd);
        }

        while (true) {
            memcpy(&active_sock_fd_set, &backup_sock_fd_set, sizeof(fd_set));
            select(sock_max_fd + 1, &active_sock_fd_set, nullptr, nullptr, nullptr);

            for (auto &node : nodes) {
                int sock_fd = node->getUDPSocketFileDescriptor();
                if (sock_fd < 0) {
                    continue;
                }
                if (!FD_ISSET(sock_fd, &active_sock_fd_set)) {
                    continue;
                }

                memset(recv_buffer, 0, MAX_PACKET_BUFFER_SIZE);
                bytes_recvd = recvfrom(sock_fd, reinterpret_cast<char *>(recv_buffer), MAX_PACKET_BUFFER_SIZE, 0, reinterpret_cast<sockaddr *>(&sender_addr), reinterpret_cast<socklen_t *>(&addr_len));
                node->receivePacket(recv_buffer, bytes_recvd);
            }
        }
     });

    t.detach();
}

void Graph::dump() const
{
    std::cout << "Topology Name = " << topology_name << std::endl;
    for (const auto &node : nodes) {
        node->dump();
    }
}
