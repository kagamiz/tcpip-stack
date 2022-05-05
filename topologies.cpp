/**
 * @file topologies.cpp
 * @author Jayson Sho Toma
 * @brief constructs topology by using graph APIs.
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

#include "Layer2/layer2.hpp"

Graph *build_first_topo()
{
    Graph *topo = new Graph("Hello World Generic Graph");
    Node *R0_re = topo->addNode("R0_re");
    Node *R1_re = topo->addNode("R1_re");
    Node *R2_re = topo->addNode("R2_re");

    topo->insertLinkBetweenTwoNodes(R0_re, R1_re, "eth0/0", "eth0/1", 1);
    topo->insertLinkBetweenTwoNodes(R1_re, R2_re, "eth0/2", "eth0/3", 1);
    topo->insertLinkBetweenTwoNodes(R0_re, R2_re, "eth0/4", "eth0/5", 1);

    R0_re->setLoopbackAddress("122.1.1.0");
    R0_re->setInterfaceIPAddress("eth0/4", "40.1.1.1", 24);
    R0_re->setInterfaceIPAddress("eth0/0", "20.1.1.1", 24);

    R1_re->setLoopbackAddress("122.1.1.1");
    R1_re->setInterfaceIPAddress("eth0/1", "20.1.1.2", 24);
    R1_re->setInterfaceIPAddress("eth0/2", "30.1.1.1", 24);

    R2_re->setLoopbackAddress("122.1.1.2");
    R2_re->setInterfaceIPAddress("eth0/3", "30.1.1.2", 24);
    R2_re->setInterfaceIPAddress("eth0/5", "40.1.1.2", 24);

    topo->startPacketReceiverThread();

    return topo;
}

Graph *build_linear_topo()
{
    Graph *topo = new Graph("Linear Topo");
    Node *H1 = topo->addNode("H1");
    Node *H2 = topo->addNode("H2");
    Node *H3 = topo->addNode("H3");

    topo->insertLinkBetweenTwoNodes(H1, H2, "eth0/1", "eth0/2", 1);
    topo->insertLinkBetweenTwoNodes(H2, H3, "eth0/3", "eth0/4", 1);

    H1->setLoopbackAddress("122.1.1.1");
    H2->setLoopbackAddress("122.1.1.2");
    H3->setLoopbackAddress("122.1.1.3");

    H1->setInterfaceIPAddress("eth0/1", "10.1.1.1", 24);
    H2->setInterfaceIPAddress("eth0/2", "10.1.1.2", 24);
    H2->setInterfaceIPAddress("eth0/3", "20.1.1.2", 24);
    H3->setInterfaceIPAddress("eth0/4", "20.1.1.1", 24);

    topo->startPacketReceiverThread();

    return topo;
}

Graph *build_simple_l2_switch_topo()
{
    Graph *topo = new Graph("Simple L2 Switch Demo graph");
    Node *H1 = topo->addNode("H1");
    Node *H2 = topo->addNode("H2");
    Node *H3 = topo->addNode("H3");
    Node *H4 = topo->addNode("H4");
    Node *L2SW = topo->addNode("L2SW");

    topo->insertLinkBetweenTwoNodes(H1, L2SW, "eth0/5", "eth0/4", 1);
    topo->insertLinkBetweenTwoNodes(H2, L2SW, "eth0/8", "eth0/3", 1);
    topo->insertLinkBetweenTwoNodes(H3, L2SW, "eth0/6", "eth0/2", 1);
    topo->insertLinkBetweenTwoNodes(H4, L2SW, "eth0/7", "eth0/1", 1);

    H1->setLoopbackAddress("122.1.1.1");
    H1->setInterfaceIPAddress("eth0/5", "10.1.1.2", 24);

    H2->setLoopbackAddress("122.1.1.2");
    H2->setInterfaceIPAddress("eth0/8", "10.1.1.4", 24);

    H3->setLoopbackAddress("122.1.1.3");
    H3->setInterfaceIPAddress("eth0/6", "10.1.1.1", 24);

    H4->setLoopbackAddress("122.1.1.4");
    H4->setInterfaceIPAddress("eth0/7", "10.1.1.3", 24);

    nodeSetInterfaceL2Mode(L2SW, "eth0/1", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/2", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/3", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/4", InterfaceNetworkProperty::L2Mode::ACCESS);

    topo->startPacketReceiverThread();

    return topo;
}
