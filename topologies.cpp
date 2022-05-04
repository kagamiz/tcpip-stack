/**
 * @file topologies.cpp
 * @author Jayson Sho Toma
 * @brief constructs topology by using graph APIs.
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

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
