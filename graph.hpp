#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <list>
#include <string>

// forward declaration
struct Node;
struct Link;

class Interface {
public:

    Node *getNode()
    {
        return att_node;
    }

    Node *getNeighbourNode()
    {
        // error handling
        if (!att_node) {
            return nullptr;
        }
        if (!link) {
            return nullptr;
        }

        // regular cases
        if (link->intf1.getNode() == att_node) {
            return link->intf2.getNode();
        }
        if (link->intf2.getNode() == att_node) {
            return link->intf1.getNode();
        }

        // irregular case
        return nullptr;
    }

private:
    std::string if_name;
    Node *att_node;
    Link *link;
};

struct Node {
public:
    Node() {}
    explicit Node(std::string name) : node_name(name)
    {
        if (node_name.length() > MAX_INTF_NAME_LENGTH) {
            // TODO: error handling
        }
        std::fill(begin(intfs), end(intfs), nullptr);
    }
    int32_t getNodeInterfaceAvailableSlot()
    {
        auto result = std::find_if(begin(intfs), end(intfs), [](Interface *p) -> bool {return !p;});
        if (result == end(intfs)) {
            return -1;
        }
        else {
            return static_cast<int32_t>(result - begin(intfs));
        }
    }

private:
    static constexpr uint32_t MAX_INTF_PER_NODE = 10;
    static constexpr uint32_t MAX_INTF_NAME_LENGTH = 16;

public:
    std::string node_name;
    std::array<Interface *, MAX_INTF_PER_NODE> intfs;
};

struct Link {
    Interface intf1, intf2;
    uint32_t cost;
};

class Graph {
public:
    Graph() {}
    explicit Graph(std::string name) : topology_name(name)
    {
        if (topology_name.length() > MAX_TOPOLOGY_NAME_LENGTH) {
            // TODO: error handling
        }
    }

private:
    std::string topology_name;
    std::list<Node *> nodes;
    static constexpr uint32_t MAX_TOPOLOGY_NAME_LENGTH = 32;
};