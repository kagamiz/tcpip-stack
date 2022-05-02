#include "graph.hpp"

Node *Interface::getNode()
{
    return att_node;
}


Node *Interface::getNeighbourNode()
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

