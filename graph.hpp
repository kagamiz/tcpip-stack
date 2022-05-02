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
    /**
     * returns the network topology node which uses this interface
     * @return the network topology node which uses this interface
     */
    Node *getNode();

    /**
     * returns the network topology node which is on the other side of this interface
     * @return the network topology node which is on the other side of this interface
     */
    Node *getNeighbourNode();

private:
    std::string if_name;
    Node *att_node;
    Link *link;
};

struct Node {
public:
    explicit Node(std::string name) : node_name(name)
    {
        if (node_name.length() > MAX_INTF_NAME_LENGTH) {
            // TODO: error handling
        }
        std::fill(begin(intfs), end(intfs), nullptr);
    }

    /**
     * returns first available slots in the interface slot list.
     * returns -1 if all the interface slots are used.
     * @returns 0-based index of the available slots or -1 if there are no vacant slots.
     */
    int32_t getNodeInterfaceAvailableSlot();

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
    explicit Graph(std::string name) : topology_name(name)
    {
        if (topology_name.length() > MAX_TOPOLOGY_NAME_LENGTH) {
            // TODO: error handling
        }
    }

    ~Graph()
    {
        for (auto &node : nodes) {
            delete node;
            node = nullptr;
        }
    }

    /**
     * inserts new node named `node_name` in the graph.
     * @param[in] node_name a name of the new node
     */
    Node *addNode(std::string node_name);

    /**
     * insert link between given two nodes.
     * if node_1 or node_2 is not an element of `nodes`, this insertion process will fail.
     * @param[in] node1
     * @param[in] node2
     * @param[in] from_if_name name of the interface connected to `node1`.
     * @param[in] to_if_name name of the interface connected to `node2`.
     * @param[in] cost integer cost of the link.
     * @return wheter insertion of the link has succeeded or not.
     */
    bool insertLinkBetweenTwoNodes(Node *node1, Node *node2, std::string from_if_name, std::string to_if_name, uint32_t cost);

private:
    std::string topology_name;
    std::list<Node *> nodes;
    static constexpr uint32_t MAX_TOPOLOGY_NAME_LENGTH = 32;
};