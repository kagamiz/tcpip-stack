/**
 * @file graph.hpp
 * @author Jayson Sho Toma
 * @brief graph data structure which is used to represent computer networks.
 * @version 0.1
 * @date 2022-05-03
 */

#pragma once

#include <array>
#include <cctype>
#include <list>
#include <string>

#include "printer.hpp"

 // forward declaration
class Node;
class Link;

/**
 * @class Interface
 * @brief A class which represents an interface connected to a network node.
 *        It holds following information:
 *        - an attached node
 *        - logical link
 */
class Interface : public IPrinter {
public:
    /**
     * @brief Construct a new Interface object
     *
     * @param name name of the interface
     */
    explicit Interface(const std::string &name);

    /**
     * @brief Get the name of the interface
     *
     * @return name of the interface
     */
    const std::string &getName() const
    {
        return if_name;
    }

    /**
     * @brief Sets node attached to this interface
     *
     * @param[in] node a pointer to the node
     */
    void setNode(Node *node)
    {
        att_node = node;
    }

    /**
     * @brief returns the network topology node which uses this interface
     *
     * @return the network topology node which uses this interface.
     */
    const Node *getNode() const
    {
        return att_node;
    }

    /**
     * @brief returns the network topology node which is on the other side of this interface
     *
     * @return the network topology node which is on the other side of this interface
     */
    const Node *getNeighbourNode() const;

    /**
     * @brief sets a link information to this interface
     *
     * @param link a pointer to the link
     */
    void setLink(Link *link)
    {
        this->link = link;
    }

    /**
     * @brief Get the Link object
     *
     * @return const Link*
     */
    const Link *getLink() const
    {
        return link;
    }

    /**
     * @brief outputs a detail of this interface on the standard output
     *
     */
    virtual void dump() const override;

private:
    std::string if_name;
    Node *att_node;
    Link *link;

    static constexpr uint32_t MAX_INTF_NAME_LENGTH = 16;
};

/**
 * @class Node
 * @brief A class which represents a network node. It holds the list of interfaces which is connected to the node.
 *
 */
struct Node : public IPrinter {
public:
    /**
     * @brief Construct a new Node object
     *
     * @param name name of the node
     */
    explicit Node(const std::string &name);

    /**
     * @brief Destroy the Node object.
     *        It also destroys links which are incident to this node.
     *
     */
    ~Node();

    /**
     * @brief Get the node name
     *
     * @return name of the node
     */
    const std::string &getName() const
    {
        return node_name;
    }

    /**
     * @brief returns an index of the first available slots in the interface slot list.
     *        returns -1 if all the interface slots are used.
     * @return 0-based index of the available slots or -1 if there are no vacant slots.
     */
    int32_t getNodeInterfaceAvailableSlot();

    /**
     * @brief returns wheter there are vacant slots to connect an interface.
     *
     * @return true if there are vacant slots
     * @return false if all the slots are occupied
     */
    bool hasVacantInterfaceSlot() const;

    /**
     * @brief try to add new interface to the interface list
     *
     * @param intf new interface
     * @return true if `intf` is added to the interface list
     * @return false if insertion fails
     */
    bool trySetInterfaceToSlot(Interface *intf);

    /**
     * @brief try to remove interface from the interface list.
     *        it only removes the reference, and will NOT free the allocated memory.
     *
     * @param intf the interface which is intended to be removed from the list
     * @return true if removal succeeds
     * @return false if removal fails
     */
    bool tryRemoveInterfaceFromSlot(Interface *intf);

    /**
     * @brief Get the interface from the interface list by name of the interface
     *
     * @param if_name name of the interface to be searched
     * @return Interface with the name specified by input parameter.
     *         Returns nullptr when there are no interface specified by given parameter on the interface list.
     */
    Interface *getNodeInterfaceByName(const std::string &if_name);

    /**
     * @brief outputs a detail of this node on the standard output
     *
     */
    virtual void dump() const override;

private:
    static constexpr uint32_t MAX_INTF_PER_NODE = 10;
    static constexpr uint32_t MAX_NODE_NAME_LENGTH = 16;

public:
    std::string node_name;
    // interface list
    std::array<Interface *, MAX_INTF_PER_NODE> intfs;
};

/**
 * @class Link
 * @brief A class which represents a logical link.
 *        It holds the information of two interfaces which is the endpoints of this link.
 *
 */
class Link {
public:

    /**
     * @brief Destroy the Link object.
     *        It also removes from(to)-interface from the from(to)-interface list of attached node.
     *
     */
    ~Link();

    /**
     * @brief tries to create new link from the given information.
     *        caller of this function is responsible for deleting allocated link.
     * @param node1
     * @param node2
     * @param from_if_name name of the interface connected to `node1`
     * @param to_if_name name of the interface connected to `node2`
     * @param cost cost of the link
     * @return Pointer to the created link. Returns nullptr if creation fails.
     */
    static Link *tryCreate(Node *node1, Node *node2, const std::string &from_if_name, const std::string &to_if_name, uint32_t cost);

    /**
     * @brief Get the From Interface object
     *
     * @return Interface*
     */
    Interface *getFromInterface()
    {
        return &intf1;
    }

    /**
     * @brief Get the To Interface object
     *
     * @return Interface*
     */
    Interface *getToInterface()
    {
        return &intf2;
    }

    /**
     * @brief Get the Cost object
     *
     * @return uint32_t
     */
    uint32_t getCost() const
    {
        return cost;
    }

private:
    /**
     * @brief Construct a new Link object
     *
     * @param from_if_name name of the interface
     * @param to_if_name name of the interface
     * @param _cost cost of the link
     */
    Link(const std::string &from_if_name, const std::string &to_if_name, uint32_t _cost);
    Interface intf1, intf2;
    uint32_t cost;
};

/**
 * @class Graph
 * @brief A class which represents a graph data structure of the network topology.
 *
 */
class Graph : public IPrinter {
public:
    /**
     * @brief Construct a new Graph object
     *
     * @param name name of the topology
     */
    explicit Graph(const std::string &name);

    /**
     * @brief Destroy the Graph object
     *
     */
    ~Graph();

    /**
     * inserts new node named `node_name` in the graph.
     * @param[in] node_name a name of the new node.
     */
    Node *addNode(const std::string &node_name);

    /**
     * insert link between given two nodes.
     * if node1 or node2 is not a node of this graph, this insertion process will fail.
     * @param node1
     * @param node2
     * @param from_if_name name of the interface connected to `node1`.
     * @param to_if_name name of the interface connected to `node2`.
     * @param cost integer cost of the link.
     * @return wheter insertion of the link has succeeded or not.
     */
    bool insertLinkBetweenTwoNodes(Node *node1, Node *node2, const std::string &from_if_name, const std::string &to_if_name, uint32_t cost);

    /**
     * @brief Get the pointer to the node by name of the node
     *
     * @param node_name name of the node
     * @return returns pointer to the node named `node_name`. Returns nullptr if such node does not exist.
     */
    Node *getNodeByNodeName(const std::string &node_name);

    /**
     * @brief outputs a detail of this graph on the standard output
     *
     */
    virtual void dump() const override;

private:
    std::string topology_name;
    std::list<Node *> nodes;
    static constexpr uint32_t MAX_TOPOLOGY_NAME_LENGTH = 32;
};
