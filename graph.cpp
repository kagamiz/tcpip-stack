/**
 * @file graph.cpp
 * @author Jayson Sho Toma
 * @brief graph data structure which is used to represent computer networks.
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

#include <algorithm>
#include <iostream>
#include <random>

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
    node_name(name.substr(0, MAX_NODE_NAME_LENGTH))
{
    std::fill(begin(intfs), end(intfs), nullptr);
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
    auto result = std::find_if(begin(intfs), end(intfs), [](Interface *p) -> bool {return !p;});
    if (result == end(intfs)) {
        return -1;
    }
    else {
        return static_cast<int32_t>(result - begin(intfs));
    }
}

bool Node::hasVacantInterfaceSlot() const
{
    auto result = std::find_if(begin(intfs), end(intfs), [](Interface *p) -> bool {return !p;});
    return result != end(intfs);
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
    auto result = std::find_if(begin(intfs), end(intfs), [&](Interface *intf) {return intf->getName() == if_name;});
    if (result == end(intfs)) {
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
    if (std::find(begin(nodes), end(nodes), node1) == end(nodes)) {
        return false;
    }
    if (std::find(begin(nodes), end(nodes), node2) == end(nodes)) {
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
    auto result = std::find_if(begin(nodes), end(nodes), [&](Node *node) -> bool { return node->getName() == node_name;});
    if (result == end(nodes)) {
        return nullptr;
    }
    return *result;
}

void Graph::dump() const
{
    std::cout << "Topology Name = " << topology_name << std::endl;
    for (const auto &node : nodes) {
        node->dump();
    }
}
